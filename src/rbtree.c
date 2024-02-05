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
  node = NULL;
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
  node_t *y = dir ? x->left : x->right;
  node_t *beta = dir ? y->right : y->left;

  // 1.1 x의 부모 <- y
  y->parent = x->parent;
  // 1.2 parent -> y (x가 루트인 경우)
  if (x == t->root)
    t->root = y;
  else if (x == x->parent->left) // x가 왼쪽 자식인 경우
    x->parent->left = y;
  else // x가 오른쪽 자식인 경우
    x->parent->right = y;

  // 2. x <-> y의 자식 (x의 자식으로 y의 자식이 오게끔)
  if (beta != t->nil)
    beta->parent = x;
  dir ? (x->left = beta) : (x->right = beta);

  // 3. x <-> y (y의 자식으로 x가 오게끔)
  dir ? (y->right = x) : (y->left = x);
  x->parent = y;
}

// a와 b의 색을 교환
void exchange_color(node_t *a, node_t *b)
{
  int tmp = a->color;
  a->color = b->color;
  b->color = (tmp == RBTREE_BLACK) ? RBTREE_BLACK : RBTREE_RED;
}
// b와 c의 색을 a로 변경하고 a의 색을 b로 변경
void recoloring(node_t *a, node_t *b, node_t *c)
{
  int tmp = a->color;
  a->color = b->color;
  b->color = c->color = tmp;
}

void rbtree_insert_fixup(rbtree *t, node_t *x)
{
  node_t *uncle;
  while (x->parent->color == RBTREE_RED)
  {
    // x의 부모가 왼쪽 자식인 경우
    if (x->parent == x->parent->parent->left)
    {
      uncle = x->parent->parent->right;
      // [CASE #1]: x의 삼촌이 적색인 경우
      if (uncle->color == RBTREE_RED)
      {
        recoloring(x->parent->parent, x->parent, uncle);
        x = x->parent->parent;
      }
      else
      {
        // [CASE #2]: x의 삼촌이 흑색이며, x가 오른쪽 자식인 경우
        if (x == x->parent->right)
        {
          x = x->parent;
          rotate(t, x, LEFT);
        }
        // [CASE #3]: x의 삼촌이 흑색이며, x가 왼쪽 자식인 경우
        exchange_color(x->parent->parent, x->parent);
        rotate(t, x->parent->parent, RIGHT);
      }
    }
    // x의 부모가 오른쪽 자식인 경우
    else
    {
      uncle = x->parent->parent->left;
      if (uncle->color == RBTREE_RED)
      {
        recoloring(x->parent->parent, x->parent, uncle);
        x = x->parent->parent;
      }
      else
      {
        if (x == x->parent->left)
        {
          x = x->parent;
          rotate(t, x, RIGHT);
        }
        exchange_color(x->parent->parent, x->parent);
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
  if (t->root == t->nil)
    return NULL;
  node_t *current = t->root;
  while (current->left != t->nil)
    current = current->left;
  return current;
}

node_t *rbtree_max(const rbtree *t)
{
  if (t->root == t->nil)
    return NULL;
  node_t *current = t->root;
  while (current->right != t->nil)
    current = current->right;
  return current;
}

// 노드 u를 노드 v로 대체
void rb_transplant(rbtree *t, node_t *u, node_t *v)
{
  if (u->parent == t->nil)
    t->root = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;
  v->parent = u->parent;
}

void rbtree_delete_fixup(rbtree *t, node_t *x)
{
  while (x != t->root && x->color == RBTREE_BLACK)
  {
    // CASE 1 ~ 4 : LEFT CASE
    if (x == x->parent->left)
    {
      node_t *w = x->parent->right;

      // CASE 1 : x의 형제 w가 적색인 경우
      if (w->color == RBTREE_RED)
      {
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        rotate(t, x->parent, LEFT);
        w = x->parent->right;
      }

      // CASE 2 : x의 형제 w는 흑색이고 w의 두 지식이 모두 흑색인 경우
      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK)
      {
        w->color = RBTREE_RED;
        x = x->parent;
      }

      // CASE 3 : x의 형제 w는 흑색, w의 왼쪽 자식은 적색, w의 오른쪽 자신은 흑색인 경우
      else
      {
        if (w->right->color == RBTREE_BLACK)
        {
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          rotate(t, w, RIGHT);
          w = x->parent->right;
        }

        // CASE 4 : x의 형제 w는 흑색이고 w의 오른쪽 자식은 적색인 경우
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        rotate(t, x->parent, LEFT);
        x = t->root;
      }
    }
    // CASE 5 ~ 8 : RIGHT CASE
    else
    {
      node_t *w = x->parent->left;

      // CASE 5 : x의 형제 w가 적색인 경우
      if (w->color == RBTREE_RED)
      {
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        rotate(t, x->parent, RIGHT);
        w = x->parent->left;
      }

      // CASE 6 : x의 형제 w는 흑색이고 w의 두 지식이 모두 흑색인 경우
      if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK)
      {
        w->color = RBTREE_RED;
        x = x->parent;
      }

      // CASE 7 : x의 형제 w는 흑색, w의 왼쪽 자식은 적색, w의 오른쪽 자신은 흑색인 경우
      else
      {
        if (w->left->color == RBTREE_BLACK)
        {
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          rotate(t, w, LEFT);
          w = x->parent->left;
        }

        // CASE 8 : x의 형제 w는 흑색이고 w의 오른쪽 자식은 적색인 경우
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        rotate(t, x->parent, RIGHT);
        x = t->root;
      }
    }
  }

  x->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *p)
{
  // TODO: implement erase
  node_t *y;
  node_t *x;
  color_t yOriginalColor;

  y = p;
  yOriginalColor = y->color;

  if (p->left == t->nil)
  {
    x = p->right;
    rb_transplant(t, p, p->right);
  }
  else if (p->right == t->nil)
  {
    x = p->left;
    rb_transplant(t, p, p->left);
  }
  else
  {
    y = p->right;
    while (y->left != t->nil)
    {
      y = y->left;
    }
    yOriginalColor = y->color;
    x = y->right;

    if (y->parent == p)
    {
      x->parent = y;
    }
    else
    {
      rb_transplant(t, y, y->right);
      y->right = p->right;
      y->right->parent = y;
    }

    rb_transplant(t, p, y);
    y->left = p->left;
    y->left->parent = y;
    y->color = p->color;
  }

  if (yOriginalColor == RBTREE_BLACK)
  {
    rbtree_delete_fixup(t, x);
  }

  free(p);

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
