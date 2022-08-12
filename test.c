#include "mm.h"


typedef struct Student
{
	char name[32];
	int id;
}student;


typedef struct Player
{
	char name[32];
	int goals;
	int assists;
	char club[16];
}player;

vm_page_family_t* lookup_page_famili(char *name);



/*
meta_block* mm_allocate_free_data_block(vm_page_family_t *vm_page_family,int units)
{
        vm_page *vm_pg = vm_page_family->first_page;

        
        meta_block *meta_block_data;
        while(vm_pg)
        {
		meta_block_data = &vm_pg->meta_block_data;		
        	while(meta_block_data)
		{
			if(meta_block_data->block_size >= units)
			{
				return meta_block_data;
			}
			meta_block_data = meta_block_data->next_block;
		}
		vm_pg=vm_pg->next;
	}
	vm_pg = allocate_vm_page(vm_page_family);
	MARK_VM_PAGE_EMPTY(vm_pg);
	return(allocate_free_data_block(vm_page_family,units));
	//printf("%d\n\n",vm_page_family->first_page->meta_block_data.block_size);
	//return NULL;
}*/

int main()
{
	mm_init();
	MM_REG_STRUCT(student);
	MM_REG_STRUCT(player);
	
	xcalloc("student",1);
	xcalloc("player",1);
	print_vm_page_for_families();

	xcalloc("student",1);
	xcalloc("student",1);
	//xcalloc("player",1);
	//xcalloc("player",1);
	//xcalloc("player",1);
	//xcalloc("player",1);	
	//xcalloc("student",1);	
	//xcalloc("student",1);

	print_memory_usage();

	/*
	vm_page_family_t *p = lookup_page_famili("player");
        if(!p)
        {
                printf("Not found !! \n");
        }else
        {
                printf("Size : %d \n",p->struct_size);
        }

	//vm_page *vm_pg = p->first_page;
	//meta_block *m_block = &vm_pg->meta_block_data;
	//printf("%d\n%p\n", m_block->block_size,m_block->next_block);
	//meta_block *m2_block = m_block->next_block;
	//printf("%d\n",m2_block->block_size);
	

	print_memory_usage();
	//test();
	/*mm_instantiate_new_page_family("struct2",10);
	mm_instantiate_new_page_family("struct1",10);
	mm_instantiate_new_page_family("struct9",10);	
	mm_instantiate_new_page_family("struct8",10);
	mm_instantiate_new_page_family("struct7",10);
	mm_instantiate_new_page_family("struct6",14);
	mm_instantiate_new_page_family("struct5",10);
	mm_instantiate_new_page_family("struct4",10);	
	mm_instantiate_new_page_family("struct3",10);
	
	print_vm_page_for_families();
	//printf("\n\n");
	
	vm_page_family_t *p = lookup_page_famili("struct6");
	if(!p)
	{
		printf("Not found !! \n");
	}else
	{
		printf("Size : %d \n",p->struct_size);
	}

	meta_block *m = allocate_free_data_block(p,28);
	if(!m)
	{
		printf("NULL\n");
	}
	
	void *ptr= xcalloc("struct8",36);
	if(!ptr)
	{
		printf("Couldn't allocate\n");
	}
	//printf("%d\n",m->block_size);
	*/
	return 0;

}



vm_page_family_t* lookup_page_famili(char *name)
{
	vm_page_for_families_t *vm_pg_families_curr = first_vm_page_for_families;
	while(vm_pg_families_curr)
	{
		for(int i=0;i<31;i++)
		{
			if(!strncmp(vm_pg_families_curr->vm_page_family[i].struct_name,name,sizeof(name)))
			{
				return &vm_pg_families_curr->vm_page_family[i];
			}
		}
		vm_pg_families_curr=vm_pg_families_curr->next;
	}	
	return NULL;
}

