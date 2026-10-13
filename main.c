/* Anti-credit card fraud:
 *
 * This code is written by Chen Jiang, original coursework implementation
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define STAGE_NUM_ONE 1						/* stage numbers */ 
#define STAGE_NUM_TWO 2
#define STAGE_NUM_THREE 3
#define STAGE_NUM_FOUR 4

#define MAX_CARD 100						 /* maximum number of cards */
#define MIN_CARD 1						     /* minimum  number of cards */
#define CARDID_LENTH 8                       /* The length of cardsID */
#define TRANSID_LENTH 12	                 /* The length of transaction ID */
#define ADD_FOR_STOP 1                       /* add an extra slot for '\0' */
#define BRKDAY 1                             /* break daily limit */
#define BRKTRAN 2                            /* break transaction limit */
#define ALLBRK 3                             /* all break */
#define NOBRK 0                              /* no break limits */
#define INFINITE 3                           /* number for a infinite loop*/
/****************************************************************/
/*define the structures*/
typedef struct transation transation_t; 

typedef struct{
	/* add an extra slot for '\0'*/
	char card_ID[CARDID_LENTH+ADD_FOR_STOP];
	int day_limit;
	int tran_limit;
	long long accumulative;
    int last_year, last_month, last_day;
}creditcard_t;

typedef struct{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int seconds;
} time_t;

struct transation{
	/* add an extra slot for '\0'*/
	char transation_ID[TRANSID_LENTH+ADD_FOR_STOP];
	char card_ID[CARDID_LENTH+ADD_FOR_STOP];
    time_t time;
	int amount;
	int status;
	transation_t *next;
};

typedef struct{
	transation_t *head;
	transation_t *foot;
} list_t;

typedef struct{
	/* add an extra slot for '\0'*/
	char transation_ID[TRANSID_LENTH+ADD_FOR_STOP];
	int status;
}detective_t;

/****************************************************************/
/* function prototypes for reading and searching*/
void read_card(creditcard_t card_arry[MAX_CARD],int *count);
void read_transaction(list_t *list, int *count, 
    creditcard_t *card_arry, int cardcount);
void get_trans(detective_t *info,list_t *list);
void time_recoder(char c, int *num, transation_t *new, int *newswich);
int compare_card_content(const void *data1, const void *data2);
int binary_search(creditcard_t A[], int lo, int hi, 
    creditcard_t* key, int *locn);

/*function prototypes for the linklist*/
void free_list(list_t *list);
list_t *make_empty_list(void);
int is_empty_list(list_t *list);
void get_tranID(detective_t *detec,list_t *list,int num);
void get_status(detective_t *detec,list_t *list,int num);
list_t *get_tail(list_t *list); 

/*function prototypes for the each stage*/
void print_stage_header(int stage_num);
void stage_one(creditcard_t card_arry[MAX_CARD]); 
void stage_two(creditcard_t card_arry[MAX_CARD], int count);
void stage_three(detective_t *info, int size);
void stage_four(detective_t *info, int size);

/****************************************************************/
/* main function controls all the action*/
int main(void){
   /*read the credit cards*/
   creditcard_t card_arry[MAX_CARD];
   int card_count = 0;
   int trans_count = 0;
   read_card(card_arry, &card_count);

   stage_one(card_arry);
   stage_two(card_arry, card_count);
   
   /*create a linklist*/
   list_t *list;
   list = make_empty_list();
   /*read the transactions*/
   /* Sort before the original binary-search-based limit checks. */
   qsort(card_arry, card_count, sizeof(creditcard_t), compare_card_content);
   read_transaction(list,&trans_count,card_arry,card_count);
   /* for initialise an arry, we need a const value*/
   const int size = trans_count;
   detective_t *info = size ? malloc((size_t)size * sizeof(*info)) : NULL;
   if (size && !info) { free_list(list); return EXIT_FAILURE; }
   get_trans(info,list);

   stage_three(info,size);
   stage_four(info,size);

   /* free memories*/
   free_list(list);
   free(info);
   list = NULL;
   return 0;
}

/****************************************************************/
/* Bound records before parsing; reject truncation and integer overflow. */
static int read_line(char *line, size_t size) {
    if (!fgets(line, (int)size, stdin)) return ferror(stdin) ? -1 : 0;
    if (!strchr(line, '\n') && !feof(stdin)) return -1;
    return 1;
}

static int nonnegative_integer(const char *text, int *value) {
    char *end;
    errno = 0;
    if (!*text || !isdigit((unsigned char)*text)) return 0;
    long n = strtol(text, &end, 10);
    if (errno || *end || n < 0 || n > INT_MAX) return 0;
    *value = (int)n;
    return 1;
}

static int identifier(const char *text, size_t length) {
    if (strlen(text) != length) return 0;
    for (size_t i=0; i<length; i++) if (!isalnum((unsigned char)text[i])) return 0;
    return 1;
}

static int valid_time(const char *text, time_t *when) {
    if (strlen(text) != 19) return 0;
    for (int i=0; i<19; i++) {
        if (i==4 || i==7 || i==10 || i==13 || i==16) { if (text[i]!=':') return 0; }
        else if (!isdigit((unsigned char)text[i])) return 0;
    }
    if (sscanf(text, "%4d:%2d:%2d:%2d:%2d:%2d", &when->year, &when->month,
        &when->day, &when->hour, &when->minute, &when->seconds) != 6) return 0;
    int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (when->year<1 || when->month<1 || when->month>12 || when->hour>23 ||
        when->minute>59 || when->seconds>59) return 0;
    if (when->year%4==0 && (when->year%100!=0 || when->year%400==0)) days[1]=29;
    return when->day>=1 && when->day<=days[when->month-1];
}

/*read the cards*/
void read_card(creditcard_t card_arry[MAX_CARD], int *count){
    char line[256], id[64], daily[64], limit[64], extra;
    int result, separator=0;
    while ((result=read_line(line, sizeof(line))) == 1) {
        if (line[0]=='%') {
            size_t n=strspn(line, "%");
            if (line[n]!='\n' && line[n]!='\r' && line[n]!='\0') break;
            separator=1; break;
        }
        if (*count>=MAX_CARD || sscanf(line, "%63s %63s %63s %c", id,daily,limit,&extra)!=3 ||
            !identifier(id,CARDID_LENTH)) break;
        creditcard_t card = {0};
        if (!nonnegative_integer(daily,&card.day_limit) || !nonnegative_integer(limit,&card.tran_limit)) break;
        for (int i=0; i<*count; i++) if (!strcmp(card_arry[i].card_ID,id)) {
            fprintf(stderr,"Invalid input: duplicate card identifier\n"); exit(EXIT_FAILURE);
        }
        strcpy(card.card_ID,id);
        card_arry[(*count)++]=card;
    }
    if (!separator || result<0 || *count<MIN_CARD) {
        fprintf(stderr,"Invalid input: expected bounded card records followed by %% separator\n");
        exit(EXIT_FAILURE);
    }
}	

/* compare the card IDs for quiking sort and binary search */
int compare_card_content(const void *data1, const void *data2){
	creditcard_t *pContent1 = (creditcard_t *)data1;
	creditcard_t *pContent2 = (creditcard_t *)data2;
	/* compare the IDs of the 2 cards */
	int results = strncmp(pContent1->card_ID, pContent2->card_ID, 
		sizeof(pContent1->card_ID));
	return results;
}

/* binary search is implimented for searching cardID */
int binary_search(creditcard_t A[], int lo, int hi,  
    creditcard_t* key, int *locn){
	int mid, outcome;
	/* if key is in A, it is between A[lo] and A[hi-1] */
	if(lo>hi){
		return 0;
	}
	mid = (lo+hi)/2;
	if((outcome = compare_card_content(key, A+mid)) < 0){
	/* use recursive to impliment the binary search */	
		return binary_search(A, lo, mid-1, key, locn);
	}else if(outcome > 0){
		return binary_search(A, mid+1, hi, key, locn);
	}else{
		*locn = mid;
		return 1;
	}
}

/* check the status of the each transcation*/
int check_status(transation_t *new, creditcard_t *card_arry, int count){
    int locn;
    creditcard_t key = {0};
    strcpy(key.card_ID,new->card_ID);
    if (!binary_search(card_arry,0,count-1,&key,&locn)) return -1;
    creditcard_t *card = &card_arry[locn];
    if (card->last_year!=new->time.year || card->last_month!=new->time.month || card->last_day!=new->time.day) {
        long previous=(long)card->last_year*10000+card->last_month*100+card->last_day;
        long current=(long)new->time.year*10000+new->time.month*100+new->time.day;
        if (current<previous) return -1;
        card->accumulative=0;
        card->last_year=new->time.year; card->last_month=new->time.month; card->last_day=new->time.day;
    }
    card->accumulative+=new->amount;
    return (card->accumulative>card->day_limit ? BRKDAY : NOBRK) +
           (new->amount>card->tran_limit ? BRKTRAN : NOBRK);
}

/*read the transactions*/
void read_transaction(list_t *list, int *count, creditcard_t *card_arry, int cardcount){
    char line[256], id[64], card[64], timestamp[64], amount[64], extra;
    int result;
    while ((result=read_line(line,sizeof(line))) == 1) {
        transation_t value = {0};
        if (*count==INT_MAX || sscanf(line,"%63s %63s %63s %63s %c",id,card,timestamp,amount,&extra)!=4 ||
            !identifier(id,TRANSID_LENTH) || !identifier(card,CARDID_LENTH) ||
            !valid_time(timestamp,&value.time) || !nonnegative_integer(amount,&value.amount)) goto invalid;
        for (transation_t *node=list->head; node; node=node->next)
            if (!strcmp(node->transation_ID,id)) goto invalid;
        strcpy(value.transation_ID,id); strcpy(value.card_ID,card);
        value.status=check_status(&value,card_arry,cardcount);
        if (value.status<0) goto invalid;
        transation_t *node=malloc(sizeof(*node));
        if (!node) goto invalid;
        *node=value;
        if (list->foot) list->foot->next=node; else list->head=node;
        list->foot=node; (*count)++;
    }
    if (result<0) goto invalid;
    return;
invalid:
    fprintf(stderr,"Invalid input: malformed, duplicate, unknown-card, or backward-date transaction\n");
    free_list(list);
    exit(EXIT_FAILURE);
}

/* record the times of the transcations */
void time_recoder(char c, int* num, transation_t *new, int *newswich){
	if(c == ':'){
		(*newswich)++;
		(*num) = 0;
	}else if(*newswich == 0){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).year = (*num);
	}else if(*newswich == 1){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).month = (*num);
	}else if(*newswich == 2){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).day= (*num);
	}else if(*newswich == 3){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).hour = (*num);
	}else if(*newswich == 4){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).minute = (*num);
	}else if(*newswich == 5){
		int toint = c-'0';
		(*num) = (*num)*10+toint;
		(new->time).seconds = (*num);
	}
}

/* collect all information of the transcations we need into an arry*/
void get_trans(detective_t *info,list_t *list){
	int i = 0;
	while(!is_empty_list(list)){
		get_tranID(info,list,i);
	    get_status(info,list,i);
		i++;
		list = get_tail(list);
	}
}

/* get tanscation ID from list */
void get_tranID(detective_t *detec,list_t *list, int num) {
	assert(list!=NULL && list->head!=NULL);
	for(int i = 0; i < TRANSID_LENTH; i++){
		detec[num].transation_ID[i] = list->head->transation_ID[i];
	}
}

/* get status from list */
void get_status(detective_t *detec,list_t *list, int num){
	assert(list!=NULL && list->head!=NULL);
	detec[num].status = list->head->status;
}

/* initialise an empty linklist*/
list_t *make_empty_list(void){
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
    if (!list) { fprintf(stderr,"Allocation failed\n"); exit(EXIT_FAILURE); }
	list->head = list->foot = NULL;
	return list;
}

/* to check if a linklist is an empty list */
int is_empty_list(list_t *list){
	assert(list != NULL);
	return (list->head == NULL);
}

/* move the head pointer*/
list_t *get_tail(list_t *list){
	transation_t *oldhead;
	assert(list!=NULL && list->head!=NULL);
	oldhead = list->head;
	list->head = list->head->next;
	if(list->head==NULL){
		 /*the only list node just got deleted*/
		list->foot = NULL;
	}
	free(oldhead);
	return list;
}

/* after we finished the task, free linklist*/
void free_list(list_t *list){
	transation_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while(curr){
		prev = curr;
		curr = curr->next;
		free(prev);
        prev = NULL;
	}
	free(list);
	list = NULL;
}

/* print the header of the each stage */   
void print_stage_header(int stage_num){
	printf("=========================Stage %d=========================\n", 
        stage_num);
}

void stage_one(creditcard_t card_arry[MAX_CARD]){
	/* print stage header */
	print_stage_header(STAGE_NUM_ONE);
	/* print the information of the first credit card*/
	printf("Card ID: ");
	for(int i = 0; card_arry[0].card_ID[i] != '\0'; i++){
		printf("%c",card_arry[0].card_ID[i]);
	}
	printf("\nDaily limit: %d\n",card_arry[0].day_limit);
	printf("Transaction limit: %d\n",card_arry[0].tran_limit);
	printf("\n");
}

void stage_two(creditcard_t card_arry[MAX_CARD],int count){
	/* print stage header */
	print_stage_header(STAGE_NUM_TWO);
	printf("Number of credit cards: %d\n",count);
	double average = 0;
	int max = 0;
	int temp = 0;
	/* calculate the average daily limit 
	as well as the card with the largest transaction limit*/
	for(int i = 0; i < count ;i++){
		average = average + card_arry[i].day_limit;
		if(temp < card_arry[i].tran_limit){
		    temp = card_arry[i].tran_limit;
			max = i;
		}
	}
	average = average/count;
	printf("Average daily limit: %.2f\n",average);
	printf("Card with the largest transaction limit: ");
	for(int i = 0; card_arry[max].card_ID[i] != '\0'; i++){
		printf("%c",card_arry[max].card_ID[i]);
	}
	printf("\n\n");
}

void stage_three(detective_t* info, int size){
	/* print stage header */
	print_stage_header(STAGE_NUM_THREE);
	/* print all transcationID */
	for(int i = 0; i < size; i++){
		for(int j = 0; j < TRANSID_LENTH; j++){
			printf("%c",info[i].transation_ID[j]);
		}
		printf("\n");
	}
	printf("\n");
}

void stage_four(detective_t* info, int size){
	/* print stage header */
	print_stage_header(STAGE_NUM_FOUR);
	/*print the all detective results*/
	for(int i = 0; i < size; i++){
		for(int j = 0; j < TRANSID_LENTH; j++){
			printf("%c",info[i].transation_ID[j]);
		}
		if(info[i].status == NOBRK){
			printf("             IN_BOTH_LIMITS");
		}else if(info[i].status == BRKDAY){
			printf("             OVER_DAILY_LIMIT");
		}else if(info[i].status == BRKTRAN){
			printf("             OVER_TRANS_LIMIT");
		}else if(info[i].status == ALLBRK){
			printf("             OVER_BOTH_LIMITS");
		}
		printf("\n");
	}
}

//Thanks
//Algorithems are Fun!
//Algorithems are Fun!
//Algorithems are Fun!
