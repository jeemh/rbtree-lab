#include "rbtree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

rbtree *new_rbtree(void)
{
  rbtree *t = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));
  t->root = t->nil = nil;    // 트리의 nill과 루트를 nil 노드로 설정
  nil->color = RBTREE_BLACK; // nil 노드는 항상 검은색

  return t;
}

// 후위 순회 방식으로 RB 트리의 노드들의 메모리를 해제
void free_node(rbtree *t, node_t *node)
{
  if (node == t->nil)
    return;
  free_node(t, node->left);
  free_node(t, node->right);
  free(node);
}

void delete_rbtree(rbtree *t)
{
  free_node(t, t->root);
  free(t->nil);
  free(t);
}

typedef enum
{
  LEFT,
  RIGHT
} dir_t;

// x를 기준으로 원하는 방향으로 회전
void rotate(rbtree *t, node_t *x, dir_t dir)
{
  node_t *parent = x->parent;
  node_t *y = dir ? x->left : x->right;
  node_t *beta = dir ? y->right : y->left;

  // 1.1 parent -> y (x가 루트인 경우)
  if (x == t->root)
    t->root = y;
  else
  {
    if (x == parent->left) // x가 왼쪽 자식인 경우
      parent->left = y;
    else // x가 오른쪽 자식인 경우
      parent->right = y;
  }

  // 1.2 parent <- y
  y->parent = parent;
  // 2. x <-> y의 자식 (x의 자식으로 y의 자식이 오게끔)
  beta->parent = x;

  if (dir)
    x->left = beta;
  else
    x->right = beta;

  // 3. x <-> y (y의 자식으로 x가 오게끔)
  x->parent = y;
  if (dir)
    y->right = x;
  else
    y->left = x;
}

void rbtree_insert_fixup(rbtree *t, node_t *x)
{
  while (x->parent->color == RBTREE_RED)
  {
    // x의 부모가 왼쪽 자식인 경우
    if (x->parent == x->parent->parent->left)
    {
      node_t *uncle = x->parent->parent->right;
      // case #1: x의 삼촌이 적색인 경우
      if (uncle->color == RBTREE_RED)
      {
        x->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        x->parent->parent->color = RBTREE_RED;
        x = x->parent->parent;
      }
      else
      {
        // case #2: x의 삼촌이 흑색이며, x가 오른쪽 자식인 경우
        if (x == x->parent->right)
        {
          x = x->parent;
          rotate(t, x, LEFT);
        }
        // case #3: x의 삼촌이 흑색이며, x가 왼쪽 자식인 경우
        x->parent->color = RBTREE_BLACK;
        x->parent->parent->color = RBTREE_RED;
        rotate(t, x->parent->parent, RIGHT);
      }
    }
    // 부모가 오른쪽 자식인 경우
    else
    {
      node_t *uncle = x->parent->parent->left;
      // case #1: x의 삼촌이 적색인 경우
      if (uncle->color == RBTREE_RED)
      {
        x->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        x->parent->parent->color = RBTREE_RED;
        x = x->parent->parent;
      }
      else
      {
        // case #2: x의 삼촌이 흑색이며, x가 왼쪽 자식인 경우
        if (x == x->parent->left)
        {
          x = x->parent;
          rotate(t, x, RIGHT);
        }
        // case #3: x의 삼촌이 흑색이며, x가 오른쪽 자식인 경우
        x->parent->color = RBTREE_BLACK;
        x->parent->parent->color = RBTREE_RED;
        rotate(t, x->parent->parent, LEFT);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

// 의사 코드 기반 노드 삽입 구현
node_t *rbtree_insert(rbtree *t, const key_t key)
{
  node_t *parent = t->nil;
  node_t *current = t->root;

  while (current != t->nil)
  {
    parent = current;
    if (key < current->key)
      current = current->left;
    else
      current = current->right;
  }

  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  *new_node = (node_t){RBTREE_RED, key, parent, t->nil, t->nil};

  if (parent == t->nil)
    t->root = new_node;
  else if (new_node->key < parent->key)
    parent->left = new_node;
  else
    parent->right = new_node;

  rbtree_insert_fixup(t, new_node);

  return new_node;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
  node_t *current = t->root;
  while (current != t->nil)
  {
    if (current->key == key)
      return current;
    if (current->key < key)
      current = current->right;
    else
      current = current->left;
  }
  return NULL;
}

node_t *rbtree_min(const rbtree *t)
{
  node_t *current = t->root;
  while (current->left != t->nil)
    current = current->left;
  return current;
}

node_t *rbtree_max(const rbtree *t)
{
  node_t *current = t->root;
  while (current->right != t->nil)
    current = current->right;
  return current;
}

int rbtree_erase(rbtree *t, node_t *p)
{
  // TODO: implement erase
  return 0;
}

void inorder(const rbtree *t, node_t *node, key_t *arr, const size_t n, size_t *cnt)
{
  if (*cnt == n || node == t->nil)
    return;
  inorder(t, node->left, arr, n, cnt);
  arr[*cnt] = node->key;
  *cnt += 1;
  inorder(t, node->right, arr, n, cnt);
}
// 트리를 중위 순회하며 n개의 키를 배열 arr에 저장
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  size_t cnt = 0;
  size_t *pcnt = &cnt;

  inorder(t, t->root, arr, n, pcnt);

  return 0;
}
