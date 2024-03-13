#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "stdlib.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool()
{
    int i;
    for (i = 0; i < NR_WP; i++)
    {
        wp_pool[i].NO = i;
        wp_pool[i].is_used = false;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
    assert(free_ != NULL);
    if (head == NULL)
    {
        head = free_;
        free_ = free_->next;
        head->next = NULL;
        head->is_used = true;
        return head;
    }
    else
    {
        WP *wp = head;
        while (wp->next != NULL)
        {
            wp = wp->next;
        }
        wp->next = free_;
        free_ = free_->next;
        wp->next->next = NULL;
        wp->next->is_used = true;
        return wp->next;
    }
}

bool free_wp(int No)
{
    if (No < 0 || No >= NR_WP)
    {
        printf("no is out of range\n");
        assert(0);
    }
    WP *wp = head;
    WP *prev_wp = NULL;
    if (wp == NULL)
    {
        printf("the watchpoint does not exist");
        assert(0);
    }

    while (wp != NULL)
    {
        if (wp->NO == No)
        {
            if (prev_wp == NULL)
            {
                head = wp->next;
            }
            else
            {
                prev_wp->next = wp->next;
            }

            wp->is_used = false;
            if (wp->expr)
            {
                free(wp->expr);
                wp->expr = NULL;
            }
            wp->value = 0;
            WP *free_wp = free_;
            if (free_wp == NULL)
            {
                free_ = wp;
                wp->next = NULL;
            }
            else
            {
                while (free_wp->next != NULL)
                {
                    free_wp = free_wp->next;
                }
                free_wp->next = wp;
                wp->next = NULL;
            }
            return true;
        }
        prev_wp = wp;
        wp = wp->next;
    }
    printf("ERROR,can not find no.%d watchpoint", No);
    return false;
}

void print_wp()
{
    WP *temp = head;
    if (head == NULL)
        printf("ERROR,no head");
    while (temp != NULL)
    {
        printf("%d %s %d \n", temp->NO, temp->expr, temp->value);
        temp = temp->next;
    }
}

bool check_wp()
{
    WP *current = head;
    bool is_changed = false;
    if (!current)
        return false;
    while (current != NULL)
    {
        bool success = false;
        int evaluate_value = expr(current->expr, &success);
        if (success && current->value != evaluate_value)
        {
            printf("watchpoint %d : '%s' from %d to %d\n", current->NO, current->expr, current->value, evaluate_value);
            current->value = evaluate_value;
            is_changed = true;
        }
        current = current->next;
    }
    return is_changed;
}
