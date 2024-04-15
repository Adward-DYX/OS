/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-12 14:20:20
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-14 15:20:31
 * @FilePath: /OS/chapter9/9.3/lib/kernel/list.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "list.h"

/*初始化双向链表list*/
void list_init(struct list* list){
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

/*链表元素 elem 插入在元素 before 之前*/
void list_insert_before(struct list_elem* before, struct list_elem* elem){
    enum intr_status old_status = intr_disable();

    /*将before前驱元素的后继更新为elem，是的befor脱离链表*/
    before->prev->next = elem;

    /*更新 elem 自己的前驱结点为 before 的前驱,更新 elem 自己的后继结束、为 before ，于是 before 又回到链表*/
    elem->prev = before->prev;
    elem->next = before;

    /*更新 before 的前驱结点为 elem*/
    before->prev = elem;

    intr_set_status(old_status);
}

/*添加元素到列表队首，类似钱 push 操作*/
void list__push(struct list* plist, struct list_elem* elem){
    list_insert_before(plist->head.next,elem);  //在队头插入 elem
}

/*追加元素到链表队尾，类似队列的先进先出操*/
void list_append(struct list* plist, struct list_elem* elem){
    list_insert_before(&plist->tail,elem);
}

/*使元素pelem脱离链表*/
void list_remove(struct list_elem* pelem){
    enum intr_status old_status = intr_disable();

    pelem->prev->next = pelem->next;
    pelem->next->prev = pelem->prev;
    intr_set_status(old_status);
}

/*链表第一个元素弹出并返回，类似梭的 pop 操作*/
struct list_elem* list_pop(struct list* plist){
    struct list_elem* elem = plist->head.next;
    list_remove(elem);
    return elem;
}

/*从链表中查找元素obj_elem ，成功时返回 true ，失败时返回 false*/
bool elem_find(struct list* plist, struct list_elem* obj_elem){
    struct list_elem* elem = plist->head.next;
    while(elem!=&plist->tail){
        if(elem==obj_elem)
            return true;
        elem = elem->next;
    }
    return false;
}

/** 把列表 plist 中的每个元素 elem 和 arg 传给回调函数 func,
 * arg 给 func 用来判断 elem 是否符合条件．
 * 本函数的功能是遍历列表内所有元素，逐个判断是否有符合条件的元素。
 * 找到符合条件的元素返回元素指针，否则返回 NULL
*/

struct list_elem* list_traversal(struct list* plist, function func, int arg){
    struct list_elem* elem = plist->head.next;
    if(list_empty(plist))
        return NULL;
    
    while(elem!=&plist->tail){
        if(func(elem,arg))  //func 返回 ture ，则认为该元素在回调函数中符合条件，命中，故停止继续遍历
            return elem;    
        elem = elem->next;
    }
    return NULL;
}   

/*返回链表长度*/
uint32_t list_len(struct list* plist){
    struct list_elem* elem = plist->head.next;
    uint32_t length = 0;
    while(elem!=&plist->tail){
        length++;
        elem = elem->next;
    }
    return length;
}

/*断链表是否为空，空时返回 true ，否则返回 false*/
bool list_empty(struct list* plist){
    return (plist->head.next==&plist->tail ? true : false);
}