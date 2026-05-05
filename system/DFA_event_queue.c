#include "DFA_event_queue.h"
#include "uart.h"
#include "stdlib.h"
#include "event_handlers.h"
#include <string.h>
#include "DX_BT24.h"

static trie_node_t *trie_root = NULL;//Trie树根节点，所有命令的前缀都从根节点开始匹配

static event_queue_t *event_queue = NULL;
static event_queue_t *event_queue_tail = NULL;

static DFA_cmd_t cmd_list[] = {
    {"default", 0}, // 0
    {"led-on-", 1},  // 1
    {"led-off-", 1}, // 2
    {"beep-on-", 1}, // 3
    {"beep-off-", 1}, // 4
    {"fan-on-", 1}, // 5
    {"fan-off-", 1}, // 6
    {"fan-speed-", 1}, // 7
    {"co2-auto-", 1}, // 8
    {"co2-off-", 1}, // 9
    
};
//静态函数声明：
static void Event_Push(event_handler_t evt, int param_val, 
    event_queue_t **event_queue, event_queue_t **event_queue_tail);



/*******************************************************************************
 *********************** DFA事件解析框架*****************************************
 *******************************************************************************
 */

static void Match_Event_Handler(event_type_t evt, int param_val){

    switch(evt){
        case EVT_DEFAULT:   Event_Push(default_handler, param_val, &event_queue, &event_queue_tail); break;
        case EVT_LED_ON:    Event_Push(led_on, param_val, &event_queue, &event_queue_tail); break;
        case EVT_LED_OFF:   Event_Push(led_off, param_val, &event_queue, &event_queue_tail); break;
        case EVT_BEEP_ON:   Event_Push(beep_on, param_val, &event_queue, &event_queue_tail); break;
        case EVT_BEEP_OFF:  Event_Push(beep_off, param_val, &event_queue, &event_queue_tail); break;
        case EVT_FAN_ON:    Event_Push(fan_on, param_val, &event_queue, &event_queue_tail); break;
        case EVT_FAN_OFF:   Event_Push(fan_off, param_val, &event_queue, &event_queue_tail); break;
        case EVT_FAN_SPEED: Event_Push(fan_speed, param_val, &event_queue, &event_queue_tail); break;
        case EVT_CO2_AUTO:  Event_Push(co2_auto,param_val, &event_queue, &event_queue_tail); break;
        case EVT_CO2_OFF:   Event_Push(co2_off, param_val, &event_queue, &event_queue_tail); break;
    }
}

//==================== DFA 核心：逐字节解析 ====================
//对每个队列的所有命令进行匹配
void DFA_Match_Byte(uint8_t ch)
{
    static uint8_t first_byte_received_flag = 0;
    static uint8_t dfa_cmd_progress[CMD_COUNT] = {0};//每个队列的所有命令的当前匹配进度
    static uint8_t matched_cmd_idx = 0xFF;  // 匹配到的命令编号
    static int     matched_cmd_param_val = 0;         // 参数值

    if(first_byte_received_flag == 0){
        first_byte_received_flag = 1;
        // ERR_MSG("note the cmdline must end with");
    }
    // ============= 回车/换行 = 一条完整命令结束 =============
    if(ch == '\n'){
        // 重置所有状态
        memset(dfa_cmd_progress, 0, sizeof(dfa_cmd_progress));//重置所有命令的匹配进度
        matched_cmd_idx = 0xFF;//重置匹配到的命令编号
        matched_cmd_param_val = 0;//重置参数值
        return;
    }
    // ============= 收集完前缀，开始收集参数（如果带参数）或加入队列（不带参数或参数值收集完成） =============
    if(matched_cmd_idx < CMD_COUNT){//匹配到了第g_match_cmd_idx条命令
        //如果此条指令带参数，继续收集参数值
        if(ch >= '0' && ch <= '9'){
            matched_cmd_param_val = matched_cmd_param_val * 10 + (ch - '0');
            // 可选：限制范围 0~100
            if(matched_cmd_param_val > 100) matched_cmd_param_val = 100;
            return;//收集此条命令的全部参数
        }
        //如果此条指令不带参数，或者参数值收集完成，直接加入队列
        Match_Event_Handler((event_type_t)matched_cmd_idx, matched_cmd_param_val);
        matched_cmd_idx = 0xFF;//重置匹配到的命令编号
        matched_cmd_param_val = 0;//重置参数值
    }
    // ============= 正常匹配命令前缀 =============
    for(uint8_t i = 0; i < CMD_COUNT; i++){  // 此字符匹配遍历所有命令
        // DEBUG(ch);
        if(ch == cmd_list[i].cmd[dfa_cmd_progress[i]]){//匹配到第i条命令的当前字符
            dfa_cmd_progress[i]++;  //当前命令的匹配进度增加
            // ===================== 关键：前缀匹配完成 =====================
            if(dfa_cmd_progress[i] >= strlen(cmd_list[i].cmd)){//完全匹配到第i条命令
                matched_cmd_idx = i;    //收集匹配到的指令的参数后加入队列，无参数直接加入队列
                if(cmd_list[i].has_param){//带参数
                    matched_cmd_param_val = 0;// 进入参数收集模式
                }
                else{//不带参数，直接加入队列
                    matched_cmd_param_val = -1; //无参指令参数为-1
                }
                dfa_cmd_progress[i] = 0;    //重置第i条命令的匹配进度，此条命令匹配完成
            }
        }
        else{//不匹配
            dfa_cmd_progress[i] = 0;    //重置第i条命令的匹配进度
        }
    }
}
/*******************************************************************************
 *********************** Trie事件解析框架****************************************
 *******************************************************************************
 */
#if defined(USE_TRIE_OPTIMIZATION) && (USE_TRIE_OPTIMIZATION)      //编译开关，是否使用Trie树解析命令
// 创建新节点

static trie_node_t* trie_create_node(void) {
    trie_node_t *node = (trie_node_t*)malloc(sizeof(trie_node_t));
    if (node) {
        memset(node->children, 0, sizeof(node->children));
        node->event = EVT_DEFAULT;
        node->has_param = 0;
        node->is_end = 0;
    }
    return node;
}

// 插入命令到Trie树
static void trie_insert(const char *cmd, event_type_t event, uint8_t has_param) {
    trie_node_t *node = trie_root;
    while (*cmd) {
        uint8_t ch = (uint8_t)*cmd;
        if (!node->children[ch]) {
            node->children[ch] = trie_create_node();
        }
        node = node->children[ch];
        cmd++;
    }
    node->event = event;// \0节点存储事件类型
    node->has_param = has_param; // 是否带参数
    node->is_end = 1;  // 标记命令结束节点
}
// 优化后的DFA匹配函数
void Trie_Match_Byte(uint8_t ch) {
    static trie_node_t *current_node = NULL;  // 当前匹配节点
    static int matched_cmd_param_val = 0;
    static event_type_t pending_event = EVT_DEFAULT;
    
    // 初始化
    if (!current_node) {
        current_node = trie_root;
    }
    
    // 回车换行：重置状态
    if (ch == '\n') {
        current_node = trie_root;
        matched_cmd_param_val = 0;
        pending_event = EVT_DEFAULT;
        return;
    }
    
    // 参数收集阶段
    if (pending_event != EVT_DEFAULT) {
        if (ch >= '0' && ch <= '9') {
            matched_cmd_param_val = matched_cmd_param_val * 10 + (ch - '0');
            if (matched_cmd_param_val > 100) matched_cmd_param_val = 100;
            return;
        }
        // 参数收集完成，入队
        Match_Event_Handler(pending_event, matched_cmd_param_val);
        pending_event = EVT_DEFAULT;
        matched_cmd_param_val = 0;
        current_node = trie_root;  // 重置状态机
    }
    
    // 前缀匹配阶段
    if (current_node->children[ch]) {
        current_node = current_node->children[ch];
        // 检查是否到达命令末尾
        if (current_node->is_end) {
            pending_event = current_node->event;
            if (!current_node->has_param) {
                // 不带参数，立即入队
                Match_Event_Handler(pending_event, -1);
                pending_event = EVT_DEFAULT;
                current_node = trie_root;
            }
        }
    } else {
        // 匹配失败，重置
        current_node = trie_root;
    }
}

#endif

void trie_init(void) {
#if defined(USE_TRIE_OPTIMIZATION) && (USE_TRIE_OPTIMIZATION)
    // 初始化Trie树根节点
    trie_root = trie_create_node();
    
    // 插入所有命令
    trie_insert("led-on-", EVT_LED_ON, 1);
    trie_insert("led-off-", EVT_LED_OFF, 1);
    trie_insert("beep-on-", EVT_BEEP_ON, 1);
    trie_insert("beep-off-", EVT_BEEP_OFF, 1);
    trie_insert("fan-on-", EVT_FAN_ON, 1);
    trie_insert("fan-off-", EVT_FAN_OFF, 1);
    trie_insert("fan-speed-", EVT_FAN_SPEED, 1);
    trie_insert("co2-auto-", EVT_CO2_AUTO, 1);
    trie_insert("co2-off-", EVT_CO2_OFF, 1);
    // ... 更多命令
#endif
}
/*******************************************************************************
 *************************** 事件队列框架 ***************************************
 *******************************************************************************
 */

//将事件推送到事件队列
static void Event_Push(event_handler_t evt, int param_val, 
    event_queue_t **event_queue, event_queue_t **event_queue_tail)
{
    // 触发事件
    event_queue_t *new_node = (event_queue_t *)malloc(sizeof(event_queue_t));
    if(new_node == NULL)
    {
        ERR_MSG("Event_Push: malloc failed\n");
        return;
    }
    new_node->evt = evt;
    new_node->param_val = param_val;
    new_node->next = NULL;
    if(event_queue!= NULL && *event_queue == NULL && event_queue_tail != NULL){//队列为空
        *event_queue = new_node;
        *event_queue_tail = new_node;
    }
    else if (event_queue!= NULL && *event_queue != NULL &&
         event_queue_tail != NULL && *event_queue_tail != NULL){//队列不为空，队尾指针不为空
        (*event_queue_tail)->next = new_node;
        *event_queue_tail = new_node;
    }
    else{
        ERR_MSG("Event_Push: event_queue or event_queue_tail is NULL\n");
    }
}
// 取出事件队列头事件并运行
static int8_t Event_Pop_run(event_queue_t **event_queue, event_queue_t **event_queue_tail){
    if(event_queue != NULL && *event_queue != NULL){
        event_queue_t *head = *event_queue;
        head->evt(head->param_val);//运行队头事件
        *event_queue = (*event_queue)->next;//队头指针指向下一个节点
        if(*event_queue == NULL){//队列为空
            *event_queue_tail = NULL;//队尾指针也指向NULL
        }
        free(head);//释放出队头节点的内存
        return 1;
    }
    return 0;
}

int8_t run_event_task(void){
    return Event_Pop_run(&event_queue, &event_queue_tail);
}
