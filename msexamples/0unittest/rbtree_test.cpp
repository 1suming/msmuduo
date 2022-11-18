#include<stdio.h>
#include<string.h>
#include<stdlib.h>
 #include"msmuduo/util/rbtree.h"


struct StuA {
	int a;
};
struct StuB {
	struct StuA* p;
};
//#define RB_ROOT (struct rb_root){ (struct rb_node *)0, }

#define StuB_ROOT (struct StuB) { (struct StuA*)0,}

struct mynode {
	int key;
	struct rb_node rb;

	int val;
};
int compareKey(int key1, int key2)
{
	if (key1 < key2) {
		return -1;
	}
	else if (key1 == key2) {
		return 0;
	}
	else
		return 1;

}
struct mynode* my_search(struct rb_root* root, int key)
{
	struct rb_node* node = root->rb_node;
	while (node)
	{
		struct mynode* data = rb_entry(node, struct mynode, rb);
		int ret = compareKey(key, data->key);
		if (ret < 0) {
			node = node->rb_left;
		}
		else if (ret > 0) {
			node = node->rb_right;
		}
		else
			return data;
	}
	return NULL;
}

int my_insert(struct rb_root* root, struct mynode* data)
{
	struct rb_node** tmp = &(root->rb_node), *parent = NULL;
	//Figure out where to put new code
	while (*tmp) {
		struct mynode *pthis = rb_entry(*tmp, struct mynode, rb);

		parent = *tmp;

		int ret = compareKey(data->key, pthis->key);
		if (ret < 0) {
			tmp = &((*tmp)->rb_left);
		}
		else if (ret > 0) {
			tmp = &((*tmp)->rb_right);
		}
		else
			return -1;
	}
	//add new code and rebalance tree
	rb_link_node(&data->rb, parent, tmp);
	rb_insert_color(&data->rb, root);

	return 0;
}
void my_delete(struct rb_root* root, int key)
{
	struct mynode* data = my_search(root, key);
	if (!data) {
		fprintf(stderr, "Key Not found %d.\n", key);
		return;
	}
	rb_erase(&data->rb, root);

	free(data);

}
void traverse(struct rb_root* tree)
{
	struct rb_node* node;
	for (node = rb_first(tree); node; node = rb_next(node))
	{
		struct mynode* data = rb_entry(node, struct mynode, rb);

		printf("data key=%d,val=%d\n", data->key, data->val);
	}
}
#define NUM_NODES 32



void testFunc()
{

	//struct rb_root mytree = RB_ROOT; 
	struct rb_root root;
	root.rb_node = NULL;//初始化

	struct mynode* tn[NUM_NODES];
	//insert node from 1 to NUM_NODES
	for (int i = 0; i < NUM_NODES; i++)
	{
		tn[i] = (struct mynode*)malloc(sizeof(struct mynode));
		tn[i]->key = (i + 1) * 2;
		tn[i]->val = (i + 1) * 10;
		//int my_insert(struct rb_root* root, struct mynode* data)

		my_insert(&root, tn[i]);
	}

	

	traverse(&root);

	int searchkey = 10;
	//delete
	struct mynode* data = my_search(&root, searchkey);
	if (data) {
		printf("找到了数据 key=%d,val=%d\n", data->key, data->val);
		//然后在删除

		//void my_delete(struct rb_root* root, int key)

		my_delete(&root, searchkey);

	}
	else
	{
		printf("没有找到数据\n");
	}
	searchkey = 20;
	data = my_search(&root, searchkey);
	if (data) {
		printf("找到了数据 key=%d,val=%d\n", data->key, data->val);
		//然后在删除
		//void my_delete(struct rb_root* root, int key)
		my_delete(&root, searchkey);

	}
	else
	{
		printf("没有找到数据\n");
	}

	traverse(&root);
	struct mynode *pNode = (struct mynode*)malloc(sizeof(struct mynode));
	pNode->key = 11;
	pNode->val = 11 * 5;
	my_insert(&root, pNode);

	traverse(&root);
}
int main()
{
	///struct StuB stu = StuB_ROOT; ////<gcc不会报错，但是visual studio 2019会报错

	testFunc();

}