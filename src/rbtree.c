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

// 위에 있는 노드 x를 기준으로 원하는 방향으로 회전
void rotate(rbtree *t, node_t *x, dir_t dir)
{
  node_t *y = dir ? x->left : x->right;
  node_t *beta = dir ? y->right : y->left;

  // 1-1) y의 부모를 x의 부모로 변경
  y->parent = x->parent;
  // 1-2) x의 부모가 루트인 경우: y가 새로운 루트가 된다
  if (x == t->root)
    t->root = y;
  // x가 부모의 왼쪽 자식인 경우
  else if (x == x->parent->left)
    x->parent->left = y;
  // x가 부모의 오른쪽 자식인 경우
  else
    x->parent->right = y;

  // 2) x의 자식을 y의 자식으로 변경
  dir ? (x->left = beta) : (x->right = beta);
  if (beta != t->nil)
    beta->parent = x;

  // 3) y의 자식을 x로 변경
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

// 노드 삽입 후 불균형을 복구하는 함수
void rbtree_insert_fixup(rbtree *t, node_t *node)
{
  node_t *parent = node->parent;
  node_t *grand_parent = parent->parent;
  node_t *uncle;
  int is_left = node == parent->left; // 현재 노드가 왼쪽 자식인지 여부
  int is_parent_is_left;              // 부모가 왼쪽 자식인지 여부

  // 추가된 노드가 root 노드인 경우: 색만 변경
  if (node == t->root)
  {
    node->color = RBTREE_BLACK;
    return;
  }

  // 부모가 BLACK인 경우: 변경 없음
  if (parent->color == RBTREE_BLACK)
    return;

  is_parent_is_left = grand_parent->left == parent;
  uncle = (is_parent_is_left) ? grand_parent->right : grand_parent->left;

  // [CASE 1]: 부모와 부모의 형제가 모두 RED인 경우
  if (uncle->color == RBTREE_RED)
  {
    recoloring(grand_parent, parent, uncle);
    rbtree_insert_fixup(t, grand_parent);
    return;
  }

  if (is_parent_is_left)
  {
    if (is_left)
    // [CASE 3]: 부모의 형제가 BLACK & 부모가 왼쪽 자식 & 현재 노드가 왼쪽 자식인 경우
    {
      rotate(t, grand_parent, RIGHT);
      exchange_color(parent, parent->right);
      return;
    }
    if (!is_left)
    // [CASE 2]: 부모의 형제가 BLACK & 부모가 왼쪽 자식 & 현재 노드가 오른쪽 자식인 경우
    {
      rotate(t, parent, LEFT);
      rotate(t, grand_parent, RIGHT);
      exchange_color(node, node->right);
      return;
    }
  }
  if (!is_parent_is_left)
  {
    if (is_left)
    {
      // [CASE 3 ver2]: 부모의 형제가 BLACK & 부모가 오른쪽 자식 & 현재 노드가 왼쪽 자식인 경우
      rotate(t, parent, RIGHT);
      rotate(t, grand_parent, LEFT);
      exchange_color(node, node->left);
      return;
    }
    if (!is_left)
    {
      // [CASE 2 ver2]: 부모의 형제가 BLACK & 부모가 오른쪽 자식 & 현재 노드가 오른쪽 자식인 경우
      rotate(t, grand_parent, LEFT);
      exchange_color(parent, parent->left);
    }
  }
}

// 의사 코드 기반 노드 삽입 구현
node_t *rbtree_insert(rbtree *t, const key_t key)
{
  node_t *parent = t->nil;
  node_t *current = t->root;

  // 새 노드를 삽입할 위치 탐색
  while (current != t->nil)
  {
    parent = current;
    if (key < current->key)
      current = current->left;
    else
      current = current->right;
  }

  // 새 노드 생성
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  *new_node = (node_t){RBTREE_RED, key, parent, t->nil, t->nil};

  if (parent == t->nil)
    t->root = new_node; // 트리가 비어있으면 새 노드를 트리의 루트로 지정
  else if (new_node->key < parent->key)
    parent->left = new_node; // 새 노드를 왼쪽 자식으로 추가
  else
    parent->right = new_node; // 새 노드를 오른쪽 자식으로 추가

  // 불균형 복구
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

void rbtree_erase_fixup(rbtree *t, node_t *x)
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
        // w->color = RBTREE_BLACK;
        // x->parent->color = RBTREE_RED;
        exchange_color(x->parent, w);
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
          // w->left->color = RBTREE_BLACK;
          // w->color = RBTREE_RED;
          exchange_color(w, w->left);
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
        // w->color = RBTREE_BLACK;
        // x->parent->color = RBTREE_RED;
        exchange_color(x->parent, w);
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
          // w->right->color = RBTREE_BLACK;
          // w->color = RBTREE_RED;
          exchange_color(w, w->right);
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

// 노드를 삭제하는 함수
int rbtree_erase(rbtree *t, node_t *delete)
{
  node_t *remove; // 트리에서 없어질 노드
  node_t *remove_parent;
  node_t *replace_node; // 대체노드에 연결된 노드
  int is_black, is_left;

  // 자식이 없거나 하나만 있는 경우
  if (delete->left == t->nil)
  {
    remove = delete;
    replace_node = remove->right;
  }
  else if (delete->right == t->nil)
  {
    remove = delete;
    replace_node = remove->left;
  }
  // 자식이 둘인 경우: delete의 키를 후계자 노드의 키값으로 대체, 노드의 색은 delete의 색 유지
  else
  {
    rbtree *remove_subtree = (rbtree *)calloc(1, sizeof(rbtree));
    remove_subtree->root = delete->right; // 오른쪽에 min값
    // remove_subtree->root = delete->left; //왼쪽에 max값
    remove_subtree->nil = t->nil;        // rbtree_min의 반복문 종료조건을 만족해주기 위해서
    remove = rbtree_min(remove_subtree); // 오른쪽에 min값
    // remove = rbtree_max(remove_subtree); //왼쪽에 max값
    replace_node = remove->right; // 대체할 노드: 지워질 노드인 후계자는 항상 왼쪽 자식이 없기 때문에, 자식이 있다면 오른쪽 자식 하나뿐임
    delete->key = remove->key;    // delete의 키를 후계자 노드의 키값으로 대체 (색은 변경 X)
    free(remove_subtree);
  }

  // remove 노드 제거하기
  // remove 노드가 루트인 경우
  if (remove == t->root)
  {
    t->root = replace_node;        // 대체할 노드를 트리의 루트로 지정
    t->root->color = RBTREE_BLACK; // 루트 노드는 항상 BLACK
    free(remove);
    return 0; // 불균형 복구 함수 호출 불필요 (제거 전 트리에 노드가 하나 혹은 두개이므로 불균형이 발생하지 않음)
  }

  remove_parent = remove->parent;
  is_black = remove->color;                // remove 노드 제거 전에 지워진 노드의 색 저장
  is_left = remove_parent->left == remove; // remove가 왼쪽자식인지, 오른쪽 자식인지 저장

  // remove의 자식노드와 부모노드를 연결
  if (is_left) // remove가 왼쪽 자식이었을 경우: remove 부모의 왼쪽에 이어주기
    remove_parent->left = replace_node;
  else // remove가 오른쪽 자식이었을 경우: remove 부모의 오른쪽에 이어주기
    remove_parent->right = replace_node;

  replace_node->parent = remove_parent;
  free(remove);

  // remove 노드가 검정 노드인 경우 불균형 복구 함수 호출
  if (is_black)
    rbtree_erase_fixup(t, replace_node);
  return 0;
}

void inorder(const rbtree *t, node_t *node, key_t *arr, const size_t n, size_t *cnt)
{
  if (*cnt == n || node == t->nil)
    return;
  inorder(t, node->left, arr, n, cnt);
  arr[(*cnt)++] = node->key;
  inorder(t, node->right, arr, n, cnt);
}
// 트리를 중위 순회하며 n개의 키를 배열 arr에 저장
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  size_t cnt = 0;
  inorder(t, t->root, arr, n, &cnt);

  return 0;
}
