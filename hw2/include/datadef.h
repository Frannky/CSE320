#define ABORTSTRING "\\"
#define ABORTCHAR '\\'

#define MAXMATCHES 17

#define N_BASIC_FIELDS 8
#define OTHER -1

typedef enum  {

    R_NAME = 0, R_WORK_PHONE, R_HOME_PHONE, R_COMPANY, R_WORK_ADDRESS,
    R_HOME_ADDRESS, R_REMARKS, R_UPDATED

  } Basic_Field; // lack name and rename it ?????????????
  // where should this be used????????????????????
    
extern char *Field_Names[];  
  
/* A Rolodex entry */

typedef struct {

    char *basicfields[N_BASIC_FIELDS];
    int n_others;
    char **other_fields;

  } Rolo_Entry, *Ptr_Rolo_Entry;

  
#define get_basic_rolo_field(n,x) (((x) -> basicfields)[(n)])
#define get_n_others(x) ((x) -> n_others)  
#define get_other_field(n,x) (((x) -> other_fields)[n])
  
#define set_basic_rolo_field(n,x,s) (((x) -> basicfields[(n)]) = (s))
#define set_n_others(x,n) (((x) -> n_others) = (n))
#define incr_n_others(x) (((x) -> n_others)++)
#define set_other_field(n,x,s) ((((x) -> other_fields)[n]) = (s))

typedef struct link {

    Ptr_Rolo_Entry entry;
    int matched;
    struct link *prev;
    struct link *next;

  } Rolo_List, *Ptr_Rolo_List;


#define get_next_link(x) ((x) -> next)
#define get_prev_link(x) ((x) -> prev)
#define get_entry(x)     ((x) -> entry)
#define get_matched(x) ((x) -> matched)

#define set_next_link(x,y) (((x) -> next) = (y))
#define set_prev_link(x,y) (((x) -> prev) = (y))
#define set_entry(x,y) (((x) -> entry) = (y))
#define set_matched(x) (((x) -> matched) = 1)
#define unset_matched(x) (((x) -> matched) = 0);

extern Ptr_Rolo_List Begin_Rlist;
extern Ptr_Rolo_List End_Rlist;

#define MAXLINELEN 80
#define DIRPATHLEN 100

extern int changed;
extern int reorder_file;
extern int rololocked;


//extern char *malloc();
extern Ptr_Rolo_List new_link_with_entry();
extern int compare_links();
extern char *getenv();
extern char *ctime();
extern char *select_search_string();
extern int in_search_mode;

//rolo
extern char *rolo_emalloc();
extern char *copystr();
extern char *timestring ();
extern void user_interrupt ();
extern void user_eof ();
extern void roloexit ();
extern void save_to_disk ();
extern void save_and_exit ();
extern char *home_directory ();
extern char *homedir(), *libdir();
extern int rolo_only_to_read ();
extern void locked_action ();
extern void rolo_main ();

//io
extern int read_rolodex ();
extern void write_rolo_list ();
extern void write_rolo ();
extern void display_basic_field ();
extern void display_other_field ();
extern void summarize_entry_list ();
extern void display_field_names ();
extern void display_entry ();
extern void display_entry_for_update ();
void cathelpfile();
void any_char_to_continue ();


//rlist
extern int rlength ();
extern Ptr_Rolo_List new_link_with_entry ();
extern void rolo_insert ();
extern void rolo_delete ();
extern int compare_links ();
extern void rolo_reorder ();


//options
extern void print_short ();
extern int person_match ();
extern int find_all_person_matches ();
extern void look_for_person ();
extern void print_people ();
extern void interactive_rolo ();


//search
extern char *select_search_string ();
extern int select_field_to_search_by ();
extern int match_by_name_or_company ();
extern int match_link ();
extern int find_all_matches ();
extern void rolo_search_mode ();


//operations
extern Ptr_Rolo_List create_entry ();
extern int other_fields ();
extern int add_the_entry ();
extern void rolo_add ();
extern int entry_action ();
extern void display_list_of_entries ();
extern void rolo_peruse_mode ();

//menuanx
extern int rolo_menu_yes_no ();
extern int rolo_menu_data_help_or_abort ();
extern int rolo_menu_number_help_or_abort ();

//update
extern char *get_new_value ();
extern void rolo_update_mode ();
extern Ptr_Rolo_Entry copy_entry ();

//search
extern char *select_search_string ();
extern int select_field_to_search_by ();
extern int match_by_name_or_company ();
extern int match_link ();
extern int find_all_matches ();
extern void rolo_search_mode ();
