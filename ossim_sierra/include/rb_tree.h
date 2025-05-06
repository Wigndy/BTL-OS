#ifndef RB_TREE_H
#define RB_TREE_H

#include "common.h"

enum rb_color {
    RB_RED,
    RB_BLACK
};

struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    enum rb_color color;
    struct pcb_t *proc;  
};
 
struct rb_tree {
    struct rb_node *root;
    struct rb_node *nil;   
};
 
void rb_init(struct rb_tree *tree);
 
void rb_insert(struct rb_tree *tree, struct pcb_t *proc);
 
struct pcb_t *rb_extract_min(struct rb_tree *tree); 

void rb_remove(struct rb_tree *tree, struct pcb_t *proc);

int rb_empty(struct rb_tree *tree);

#endif