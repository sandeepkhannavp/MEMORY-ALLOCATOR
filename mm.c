#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>
#include "glthread.h"
#include <stdlib.h>

static size_t VM_PAGE_SIZE=0;

void init_func()
{
	        VM_PAGE_SIZE=getpagesize();
}


//Function to request virtual memory page/s from the kernel

static void* mm_get_vm_page_from_kernel(int units)
{
        char *vm_page=mmap(0,
	                   units*VM_PAGE_SIZE,
                           PROT_READ|PROT_WRITE|PROT_EXEC,
			   MAP_ANON|MAP_PRIVATE,
			   0,
			   0
			);

	if(vm_page == MAP_FAILED)
	{
		printf("Unable to allocate vm page\n");
		return 0;

	}
	//Inialitize the block of memory allocated with 0 if the vm page
	//allocation is successful
	                                                                                                                                                                                                                   memset(vm_page,0,units*VM_PAGE_SIZE);
																											   //return the pointer to the begining of the allocatd pages
                                                                                                                                                                                                                           return (void*)vm_page;
}
                                                                                                                                                                                                                   //Function to return the virutal memory pages to the kernel

static void* mm_return_vm_page_to_kernel(void *vm_page, int units)
{
	//munmap function to return virtual memory to the kernel
                                                                                                                                                                                                                           if(munmap(vm_page,units*VM_PAGE_SIZE))
																											   {
                                                                                                                                                                                                                           //the above unmap fiunction returns 0 if virtual memory page has been return             	                                                                                                                                                                                          
																											 	   printf("ERROR:couldn't free the virtual memory pages");                                                                                                                                                                                                          
																											   }

}






typedef struct block_meta_data
{
	//whether the data block is free
	bool isfree;\
	//the size of the data block
	int block_size;
	//offset of the meta blokc from the starting of the virtual memory page
	int offset;
	struct block_meta_data *next_block;
	struct block_meta_data *prev_block;
	//freeList *priority_node;
	//Glthread_node priority_thread_node; //(node of a linked list)
	
}meta_block;

typedef struct vm_page_family
{
      char struct_name[32];
      int struct_size;
      struct Vm_page *first_page;
      
      //freeList *head;
      
      
      //Glthread free_block_priority_list_head;

      //Glthread block_priority_list_head; //Node of a linked list for the priority queue of meta blocks oredered by size in descending
}vm_page_family_t;



typedef struct Vm_page
{
	struct Vm_page *next;
	struct Vm_page *prev;
	vm_page_family_t *pg_family; //The page family to which this virtual memory page belongs to
	meta_block meta_block_data; // the metablock which is stored above the vm page data structure in the virtual memory page

	char *page_memory; //Points to the start of the virtual memory data block
			
		
}vm_page;
			
#define MAX_FAMILIES_PER_VM_PAGE \
       	(VM_PAGE_SIZE-sizeof(vm_page_family_t*))/sizeof(vm_page_family_t)


typedef struct vm_page_for_families
{
	struct vm_page_for_families *next;
	vm_page_family_t vm_page_family[31];

}vm_page_for_families_t;






//MACROS FOR THE META DATA BLOCK

#define NEXT_META_BLOCK(meta_block_ptr)\
	(meta_block_ptr->next)
#define PREV_META_BLOCK(meta_block_ptr)\
	(meta_block_ptr->prev)
#define MM_GET_PAGE_FROM_META_BLOCK(meta_block_ptr)\
	((void*)((char*)meta_block_ptr - meta_block_ptr->offset))
#define NEXT_META_BLOCK_BY_SIZE(meta_block_ptr)\
	(meta_block*)((char*)((meta_block_ptr + 1) + meta_block_ptr->block_size))

#define mm_bind_blocks_for_allocation(allocated_meta_block,free_meta_block)\
	free_meta_block->prev_block=allocated_meta_block;\
	free_meta_block->next_block=allocated_meta_block->next_block;\
	allocated_meta_block->next_block=free_meta_block;\
	if(free_meta_block->next_block)\
	{\
		free_meta_block->next_block->prev_block=free_meta_block;\
	}



vm_page_for_families_t *first_vm_page_for_families = NULL;

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr,curr) \
{								   \
	int count=0;	\
	int i=0;\
	for(curr=(vm_page_family_t*)&vm_page_for_families_ptr->vm_page_family[i]; \
			curr->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;i++, \
			curr=(vm_page_family_t*)&vm_page_for_families_ptr->vm_page_family[i],count++)\
	{
#define ITERATE_PAGE_FAMILIES_END(vm_page_for_families_ptr,curr) }}

static bool split_free_blocl(vm_page_family_t *vm_page_family, meta_block *meta_block_data, int size);

void mm_instantiate_new_page_family(char *struct_name,int size)
{
	vm_page_family_t *vm_page_family_curr = NULL;
	vm_page_for_families_t *new_vm_page_for_families = NULL;
	
	//assuming the struct size > vm page size
	//1. If the fisrt vm_page for families is null  then we create a new vm page 
	if(!(first_vm_page_for_families))
	{
		printf("creating first vm page fro families\n");
		first_vm_page_for_families=(vm_page_for_families_t*)mm_get_vm_page_from_kernel(1);
		first_vm_page_for_families->next=NULL;
		
		strncpy(first_vm_page_for_families->vm_page_family[0].struct_name,struct_name,32);
		first_vm_page_for_families->vm_page_family[0].struct_size = size;
		first_vm_page_for_families->vm_page_family[0].first_page=NULL;
		//first_vm_page_for_families->vm_page_family[0].head=NULL;
		//Initialize_Glthread(&first_vm_page_for_families->vm_page_family[0].free_block_priority_list_head,offsetof(meta_block,priority_thread_node));
		return;
	}

	int c=0;
	ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families,vm_page_family_curr)
	{	if(strncmp(vm_page_family_curr->struct_name,struct_name,32)!=0)
		{
			c++;
			continue;
		}
		//instantiating same page family details again . => assert
		assert(0);
	}
	ITERATE_PAGE_FAMILIES_END(first_vm_page_for_families,vm_page_family_curr);

	
	//If the count var is equal to max familes for vm page then new vm page should be requested
	if(c == MAX_FAMILIES_PER_VM_PAGE)
	{
		new_vm_page_for_families= (vm_page_for_families_t*)mm_get_vm_page_from_kernel(1);
		new_vm_page_for_families->next=first_vm_page_for_families;
		first_vm_page_for_families=new_vm_page_for_families;
		vm_page_family_curr=&first_vm_page_for_families->vm_page_family[0];
	}
	
	//if count<max families per vm page then no change
	strncpy(vm_page_family_curr->struct_name,struct_name,32);
	vm_page_family_curr->struct_size=size;
	vm_page_family_curr->first_page=NULL;
	//vm_page_family_curr->head=NULL;
	//Initialize_Glthread(&vm_page_family_curr->free_block_priority_list_head,offsetof(meta_block,priority_thread_node));
	
}


#define MM_REG_STRUCT(struct_name)\
	(mm_instantiate_new_page_family(#struct_name,sizeof(struct_name)))

void print_helper()
{
	vm_page_family_t *vm_page_family=NULL;
	//vm_page_for_families *vm_
	if(!first_vm_page_for_families)
	{
		printf("NULL\n");
	}
	printf("REGISTERED PAGE FAMILIES IN VIRTUAL MEMORY  : \n");
	printf("%s\t\t%s\t\t%s\n\n","Structure Name","Structure Size","First Page ptr");
	ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families,vm_page_family)
	{
		printf("%s\t\t\t%d\t\t\t%p\n",vm_page_family->struct_name,vm_page_family->struct_size,vm_page_family->first_page);

	}
	ITERATE_PAGE_FAMILIES_END(vm_page_for_families,vm_page_family)
}


static void mm_union_free_blocks(meta_block *first,meta_block *second)
{
	if(first->isfree==false || second->isfree==false)
	{
		return;
	}

	first->block_size += sizeof(meta_block) + second->block_size;
	first->next_block = second->next_block;
	if(second->next_block)
	{
		second->next_block->prev_block = first;
	}

}


bool mm_is_vm_page_empty(vm_page *vm_pg)\
{
	if(vm_pg->meta_block_data.next_block==NULL &&
	   vm_pg->meta_block_data.prev_block==NULL &&
	   vm_pg->meta_block_data.isfree == true)
	{
		return true;
	}		
	return false;
}

//set the first meta block of the page pointer to point to NULL values for the next and prev pointers
#define MARK_VM_PAGE_EMPTY(vm_page_ptr)\
	vm_page_ptr->meta_block_data.next_block=NULL;\
	vm_page_ptr->meta_block_data.prev_block=NULL;\
	vm_page_ptr->meta_block_data.isfree=true;\
	vm_page_ptr->meta_block_data.block_size=4000;

//Iterate over the pages of a particular page family of a process
#define ITERATE_VM_PAGE_BEGIN(vm_page_family,curr)\
{\
	for(curr=vm_page_family->first_page,curr,curr=curr->next)\
	{
#define ITERATE_VM_PAGE_END(vm_page_family,curr) }}

#define ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(vm_page_ptr,curr)\
	for(curr=&vm_page_ptr->meta_block_data,curr,cur=curr->next_block)\
	{

#define ITERATE_VM_PAGE_ALL_BLOCKS_END(vm_page_ptr,curr)}}


static inline int mm_max_page_allocatable_memory(int units)
{
	return (VM_PAGE_SIZE*units - sizeof(vm_page) - sizeof(meta_block));
}



//Allocate virtual memory page from the kernel for a particular vm page family
vm_page* allocate_vm_page(vm_page_family_t *vm_page_family)
{
	vm_page *vm_pg=mm_get_vm_page_from_kernel(1);



	MARK_VM_PAGE_EMPTY(vm_pg);
	vm_pg->meta_block_data.block_size= mm_max_page_allocatable_memory(1);
	vm_pg->meta_block_data.offset=sizeof(vm_page);

	//vm_pg->meta_block_data.priority_node=NULL;
	//Initialize_Glthread_node(&vm_pg->meta_block_data.priority_thread_node);

	vm_pg->next=NULL;
	vm_pg->prev=NULL;

	vm_pg->pg_family=vm_page_family;

	//if this is the first vm page allocated to the page family
	if(vm_page_family->first_page == NULL)
	{
		vm_page_family->first_page=vm_pg;
		return vm_pg;
	}

	vm_pg->next = vm_page_family->first_page;
	vm_page_family->first_page = vm_pg;
	return vm_pg;

}

//To delete a virtual vm page that has been allocated to a vm page family
void mm_vm_page_delete_and_free(vm_page *vm_pg)
{
	vm_page_family_t *vm_page_family = vm_pg->pg_family;


	//If the vm page to delete is the first page
	if(vm_page_family->first_page = vm_pg)
	{
		vm_page_family->first_page=vm_pg->next;
		if(vm_pg->next)
		{
			vm_pg->next->prev=NULL;
		}
		vm_pg->next=NULL;
		vm_pg->prev=NULL;
		mm_return_vm_page_to_kernel((void*)vm_pg,1);
		return;
	}

	//If the virtual memory page is not the first one
	if(vm_pg->next)
	{
		vm_pg->next->prev = vm_pg->prev;

	}
	vm_pg->prev->next = vm_pg->next;
	mm_return_vm_page_to_kernel((void*)vm_pg,1);

}

static int free_blocks_comparison(void *block_meta_data1,void * block_meta_data2)
{
	meta_block *meta_block_data1 = (meta_block*)block_meta_data1;
	meta_block *meta_block_data2 = (meta_block*)block_meta_data2;

	if(meta_block_data1->block_size > meta_block_data2->block_size)
	{
		return -1;
	
	}else if(meta_block_data1->block_size < meta_block_data2->block_size)
	{
		return 1;
	}
	return 0;
	
}
/*
//to insert a free block into the priority list of free blocks for a vm page family
static void mm_add_free_block_meta_data_to_free_block_list(vm_page_family_t *vm_page_family,meta_block *free_block)
{
	if(free_block->isfree==false)
	{
		return;
	}

	freeList *temp=NULL;
	temp=vm_page_family->head;

	if(temp==NULL)
	{
		head=free_block->priority_node;
		head->prev=NULL;
		head->next=NULL;
		return;
	}

	while(temp->)
}
*/

vm_page_family_t* lookup_page_family(char *name)
{
	vm_page_for_families_t *vm_pg_families_curr = first_vm_page_for_families;
	while(vm_pg_families_curr)
	{
		for(int i=0;i<31;i++)
		{
			if(!&vm_pg_families_curr->vm_page_family[i])
			{
				break;
			}
			if(!strncmp(vm_pg_families_curr->vm_page_family[i].struct_name,name,sizeof(name)))
			{
				return &vm_pg_families_curr->vm_page_family[i];
			}
			
		}
		vm_pg_families_curr=vm_pg_families_curr->next;
	}
	return NULL;
}


meta_block* allocate_free(vm_page_family_t *vm_page_family,int units)
{
	vm_page *vm_pg = vm_page_family->first_page;
	//meta_block *meta_array[];
	meta_block *meta_block_data;
	bool split;
	while(vm_pg)
	{
		meta_block_data = &vm_pg->meta_block_data;
		while(1)
		{
			if(meta_block_data->isfree==true && meta_block_data->block_size >= units )
			{
				split=split_free_blocl(vm_page_family,meta_block_data,units);
				printf("exiting allocation\n");
				return meta_block_data;
			}
			meta_block_data = meta_block_data->next_block;
		}
		vm_pg=vm_pg->next;		
	}
	//                                                                                                              
	vm_pg = allocate_vm_page(vm_page_family);
	MARK_VM_PAGE_EMPTY(vm_pg);

	return(allocate_free(vm_page_family,units));
	//printf("%d\n\n",vm_page_family->first_page->meta_block_data.block_size);
	//return NULL;

}

static bool split_free_blocl(vm_page_family_t *vm_page_family, meta_block *meta_block_data, int size)
{

	if(meta_block_data->isfree==false)
	{
		return false;
	}

	if(meta_block_data->block_size < size)
	{
		return false;
	}
	meta_block *next_meta_block = NULL;
	
	meta_block_data->isfree=false;
	//meta_block_data->block_size=size;

	int size_remain = meta_block_data->block_size - size;
	
	meta_block_data->block_size=size;


	//CASE 1 : NO SPLIT OF DATA BLOCK
	if(size_remain==0)
	{	printf("CASE1 \n");
		return true;
	}
	printf("%d\n",size_remain);
	//CASE 2 : PARTIAL SPLIT  -  INTERNAL FRAGMENTATION
	//The remaining memory size has size only to allocate meta block but not another structure of the page family
	//printf("CASE2 \n");
	if(size_remain > sizeof(meta_block) && size_remain < (sizeof(meta_block) + vm_page_family->struct_size))
	{
		printf("CASE2 \n");

		next_meta_block = NEXT_META_BLOCK_BY_SIZE(meta_block_data);
		//next_meta_block=memset(next_meta_block,0,sizeof(meta_block));
		//memset((char*)(free_meta_block +1),0,free_meta_block->block_size)
		next_meta_block=malloc(sizeof(meta_block));
		next_meta_block->isfree=true;
		next_meta_block->block_size = size_remain - sizeof(meta_block);
		next_meta_block->offset = meta_block_data->offset + sizeof(meta_block) + meta_block_data->block_size;
		//mm_bind_blocks_for_allocation(meta_block_data,next_meta_block);
		//printf("A\n");
		next_meta_block->prev_block=meta_block_data;
		//printf("B\n");
		if(meta_block_data->next_block)
		{	//printf("C\n");
			meta_block_data->next_block->prev_block = next_meta_block;
			next_meta_block->next_block = meta_block_data->next_block; 
			//printf("D\n");
		}else
		{	//printf("E\n");
			next_meta_block->next_block=NULL;
			//printf("F\n");
		}
		//printf("G\n");
		meta_block_data->next_block=next_meta_block;
		printf("H\n");
		return true;
	}

	//CASE 3 : PARTIAL SPLIT - HARD INTERNAL FRAGMENTATION
	//Whent the remaining size isnt enough to allocate the meta block itself
	//printf("CASE3 \n");
	if(size_remain < sizeof(meta_block))
	{
		printf("CASE2 \n");
		return true;
	}

	
	else
	{
		printf("CASE4 \n");
		next_meta_block = NEXT_META_BLOCK_BY_SIZE(meta_block_data);
	  	//next_meta_block=memset(next_meta_block,,sizeof(meta_block));
		next_meta_block=(meta_block*)malloc(sizeof(meta_block));
		next_meta_block->isfree=true;
		next_meta_block->block_size = size_remain - sizeof(meta_block);
		next_meta_block->offset = meta_block_data->offset + sizeof(meta_block) + meta_block_data->block_size;
		//mm_bind_blocks_for_allocation(meta_block_data,next_meta_block);
		//printf("A\n");
	 	next_meta_block->prev_block=meta_block_data;
		//printf("B\n");
		if(meta_block_data->next_block)
		{       //printf("C\n");
			meta_block_data->next_block->prev_block = next_meta_block;
			next_meta_block->next_block = meta_block_data->next_block;
			//printf("D\n");
		}else
		{       //printf("E\n");
			next_meta_block->next_block=NULL;
			//printf("F\n");
		}
		//printf("G\n");
		meta_block_data->next_block=next_meta_block;
		printf("H\n");
	}
	
	return true;

}


void* xcalloc(char *struct_name,int units)
{
	vm_page_family_t *vm_pg_family = lookup_page_family(struct_name);
	
	if(!vm_pg_family)
	{
		printf("Page family not registered yet \n");
		return NULL;
	}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	if(units*vm_pg_family->struct_size > mm_max_page_allocatable_memory(1))
	{
		printf("Requested memory exceeds available page memory size \n");
	}


	meta_block *free_meta_block = NULL;
	free_meta_block = allocate_free(vm_pg_family,units*vm_pg_family->struct_size);
	if(free_meta_block)
	{	
		printf("Allocated\n");
		memset((char*)(free_meta_block +1),0,free_meta_block->block_size);
		printf("memset done\n");
		printf("%p\n",free_meta_block);
		return (void*)(free_meta_block + 0);
		
	}
	return NULL;
	
}


void print_memory_usage()
{
	vm_page_for_families_t *vm_page_for_families = first_vm_page_for_families;
	vm_page_family_t *vm_page_family = NULL;
	vm_page *vm_pg = NULL;
	meta_block *meta_block_data = NULL;
//iterating through the vm page families
	while(vm_page_for_families)
	{	

		for(int i=0;i<31;i++)
		{	
			vm_page_family = &vm_page_for_families->vm_page_family[i];
			
			printf("%s\t\n",vm_page_family->struct_name);
			if(!vm_page_family->first_page)
			{
				continue;
			}
			vm_pg = vm_page_family->first_page;
			
			while(vm_pg)
			{
				meta_block_data=&vm_pg->meta_block_data;
				
				while(meta_block_data)
				{
					if(meta_block_data->isfree)
					{
						printf("FREE\t");
					}else
					{
						printf("ALLOCATED\t");
					}
					printf("%d\t%d\t%p\t%p\n",meta_block_data->block_size,meta_block_data->offset,meta_block_data->next_block,meta_block_data->prev_block);

					meta_block_data=meta_block_data->next_block;
				}
				
				vm_pg=vm_pg->next;
			}	
			printf("\n\n");
		}

		
			
		vm_page_for_families=vm_page_for_families->next;
	}


}





