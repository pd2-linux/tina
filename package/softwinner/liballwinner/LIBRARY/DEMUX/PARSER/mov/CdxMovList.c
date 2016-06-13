#define LOG_TAG "CdxMovList"
#include "CdxMovList.h"

typedef struct tagIS
{
	struct tagIS 	*next;
	void* 			data;
}ItemSlot;

struct _tag_array
{
	struct tagIS 	*head;
	struct tagIS 	*tail;
	unsigned int 	entryCount;       // number of ItemSlot in the list
	int 			foundEntryNumber;
	struct tagIS 	*foundEntry;
};


void struct_list_free(AW_List* list, void(*_free)(void*))
{
	if(!list)	return;

	while(aw_list_count(list))
	{
		void* item = aw_list_last(list);
		aw_list_rem_last(list);
		if(item && _free) _free(item);
	}

	aw_list_del(list);
}




AW_List* aw_list_new()
{
	AW_List* mlist = (AW_List*)malloc(sizeof(AW_List));
	if(!mlist) return NULL;
	
	mlist->head = mlist->foundEntry = NULL;
	mlist->tail = NULL;
	mlist->foundEntryNumber = -1;
	mlist->entryCount = 0;
	return mlist;
}

int aw_list_rem(AW_List* ptr, unsigned int itemNumber)
{
	ItemSlot *tmp, *tmp2;
	if(!ptr || (!ptr->entryCount) || (!ptr->head) || (itemNumber >= ptr->entryCount)) 
		return -1;

	//delete head
	if(!itemNumber)
	{
		tmp = ptr->head;
		ptr->head = ptr->head->next;
		ptr->entryCount--;
		ptr->foundEntry = ptr->head;
		ptr->foundEntryNumber = 0;
		free(tmp);

		// that was the last entry, reset the tail
		if(!ptr->entryCount)
		{
			ptr->tail = ptr->head = ptr->foundEntry = NULL;
			ptr->foundEntryNumber = -1;
		}
		return 0;
	}

	tmp = ptr->head;
	unsigned int i;
	for(i = 0; i < itemNumber-1; i++)
	{
		tmp = tmp->next;
	}
	tmp2 = tmp->next;
	tmp->next = tmp2->next;
	/*if we deleted the last entry, update the tail !!!*/
	if (! tmp->next || (ptr->tail == tmp2) ) {
		ptr->tail = tmp;
		tmp->next = NULL;
	}
	
	free(tmp2);
	ptr->entryCount -- ;
	ptr->foundEntry = ptr->head;
	ptr->foundEntryNumber = 0;

	return 0;
}


void aw_list_del(AW_List *ptr)
{
	if (!ptr) return;
	while (ptr->entryCount) aw_list_rem(ptr, 0);
	free(ptr);
}

void aw_list_reset(AW_List *ptr)
{
	while (ptr && ptr->entryCount) aw_list_rem(ptr, 0);
}

int aw_list_add(AW_List* ptr, void* item)
{
	ItemSlot *entry;
	if(!ptr) 
	{
		printf(" parameter error. \n");
		return -1;
	}
	entry = (ItemSlot*)malloc(sizeof(ItemSlot));
	if(!entry) return -1;

	entry->data = item;
	entry->next = NULL;

	if(!ptr->head)
	{
		ptr->head = entry;
		ptr->entryCount = 1;
	}
	else
	{
		ptr->entryCount += 1;
		ptr->tail->next = entry;
	}
	ptr->tail = entry;
	ptr->foundEntryNumber = ptr->entryCount - 1;
	ptr->foundEntry = entry;
	return 0;
}

unsigned int aw_list_count(AW_List* ptr)
{
	if(!ptr) return -1;
	return ptr->entryCount;
}

void* aw_list_get(AW_List *ptr, unsigned int itemNumber)
{
	ItemSlot* entry;
	unsigned int i;
	if(!ptr || (itemNumber >= ptr->entryCount)) return NULL;
	
	//if it is the first time to get item, get the first item
	if(!ptr->foundEntry || (itemNumber<(unsigned int)ptr->foundEntryNumber))
	{
		ptr->foundEntryNumber = 0;
		ptr->foundEntry = ptr->head;
	}
	entry = ptr->foundEntry;
	for(i = ptr->foundEntryNumber; i< itemNumber; ++i)
	{
		entry = entry->next;
	}
	ptr->foundEntryNumber = itemNumber;
	ptr->foundEntry = entry;

	return (void*) entry->data; 
	
}

void* aw_list_last(AW_List* ptr)
{
	ItemSlot* entry;

	if(!ptr || !ptr->entryCount) return NULL;
	entry = ptr->head;
	
	while(entry->next)
	{
		entry = entry->next;
	}

	return entry->data;
}


int aw_list_rem_last(AW_List *ptr)
{
	return aw_list_rem(ptr, ptr->entryCount-1);
}

int aw_list_insert(AW_List *ptr, void* item, unsigned int position)
{
	if(!ptr || !item) return -1;

	//add the item to the end of list
	if(position >= ptr->entryCount) 
		return aw_list_add(ptr, item);

	ItemSlot *entry;
	entry = (ItemSlot*)malloc(sizeof(ItemSlot));
	entry->data = item;
	entry->next = NULL;

	// insert in head of list
	if (position == 0)
	{
		entry->next = ptr->head;
		ptr->head = entry;
		ptr->entryCount++;
		ptr->foundEntry = entry;
		ptr->foundEntryNumber = 0;
		return 0;
	}
	
	unsigned int i;
	ItemSlot* tmp = ptr->head;
	for(i=0; i<position-1; i++)
	{
		tmp = tmp->next;
	}
	entry->next = tmp->next;
	tmp->next = entry;
	ptr->entryCount++;
	ptr->foundEntry = entry;
	ptr->foundEntryNumber = i;
	return 0;
}

