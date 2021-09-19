/* Anti-credit card fraud:
 *
 * This code is written by Chen Jiang, StudentID:1127411
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
	int  accumulative;
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
int main(int argc, char *argv[]){
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
   read_transaction(list,&trans_count,card_arry,card_count);
   /*qsort the cardIDs for the wider usability*/
   qsort(card_arry, card_count, sizeof(creditcard_t), compare_card_content);
   /* for initialise an arry, we need a const value*/
   const int size = trans_count;
   detective_t info[size];
   get_trans(info,list);

   stage_three(info,size);
   stage_four(info,size);

   /* free memories*/
   free_list(list);
   list = NULL;
   return 0;
}

/****************************************************************/
/*read the cards*/
void read_card(creditcard_t card_arry[MAX_CARD],int *count){
	int c;
	int i = 0;
	int swich = 0;
	int num = 0;
	/* stop at %, 
	since there are just alphanumeric characters in the CardID*/	
	while((c = getchar())!= '%') {
		if(c == '\n'){
			/* count the cards we input*/
				card_arry[*count].accumulative = 0;
				card_arry[(*count)++].card_ID[i] = '\0'; 
				i = 0;
			    swich = 0;
		}else{ 
			if(c == ' '){
				num = 0;
				swich++;
			}else{
				/* read the card ID*/
				if(swich == 0){
					card_arry[*count].card_ID[i++] = c;
				/* read the daily limit*/
				}else if(swich == 1){
					int toint = c-'0';
					num = num*10+toint;
					card_arry[*count] . day_limit = num;
				/* read the transaction limit*/
				}else if(swich == 2){
					int toint = c-'0';
					num = num*10+toint;
					card_arry[*count] . tran_limit = num;
				}	
			}	
		}		
	}
	/* if the number of cards is too big, system will exit */	
		if(*count > MAX_CARD){
			printf("Invalid input, toooooooo many cards.\n");
			exit(EXIT_FAILURE);
		}
	/* if the number of cards is less than 1, system will exit */	
		if(*count < MIN_CARD){
			printf("Invalid input, there is no card.\n");
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
		return binary_search(A, lo, mid, key, locn);
	}else if(outcome > 0){
		return binary_search(A, mid+1, hi, key, locn);
	}else{
		*locn = mid;
		return 1;
	}
}

/* check the status of the each transcation*/
int check_status(transation_t *new, creditcard_t *card_arry, int count){
	int checkdaylim = NOBRK ;
	int checktranlim = NOBRK ;
	static int year;
	static int month;
	static int day;
	int locn = 0;
	/* read the current card ID of the credit card*/
	creditcard_t* newcard = (creditcard_t*)malloc(sizeof(creditcard_t));
	strncpy (newcard->card_ID, new-> card_ID, CARDID_LENTH);
	binary_search(card_arry, 0, count-1, newcard, &locn);
	int trantest = card_arry[locn].tran_limit;
	/* reset the day spending*/
	if(year != (new->time).year || month != (new->time).month 
        || day != (new->time).day){
		year = (new->time).year;
		month = (new->time).month;
		day = (new->time).day;
		card_arry[locn].accumulative = new->amount;
	}else{
		card_arry[locn].accumulative = card_arry[locn].accumulative+new->amount;
	}
	/* identify the status of this transcation*/
	if(card_arry[locn].day_limit - card_arry[locn].accumulative < 0){
		checkdaylim = BRKDAY;
	}
	if((trantest - (new->amount)) < 0){
		checktranlim = BRKTRAN;
	}	
	free(newcard);
	newcard = NULL;
	return checkdaylim + checktranlim;
}

/*read the transactions*/
void read_transaction(list_t *list, int *count, 
    creditcard_t *card_arry, int cardcount){
	int c;
	int i;
	int num;
	int swich;
	int newswich;
	transation_t *new;
	/*this infinite while loop will be stopped in the loop */
    while(INFINITE){
    	c = getchar();
		if(c == '\n'||c == EOF){
			if(swich == 3){
				/* check the status of this transcation */
				new->status = check_status(new, card_arry, cardcount);
				/* pack the nodes */
				new->next = NULL;
				if(list->foot==NULL){
					/* this is the first insertion into the list */
					list->head = list->foot = new;
				}else{
					list->foot->next = new;
					list->foot = new;
				}
				(*count)++;
			}
			/* stop the loop when we read EOF*/
			if(c == EOF){
				break;
			}
			/* apply a memory space for the current node*/
			new = (transation_t*)malloc(sizeof(*new));
			assert(list!=NULL && new!=NULL);
			swich = 0;
			newswich = 0;
			i = 0;
		}else if(c != '%'){ 
			if(c == ' '){
				i = 0;
				num = 0;
				swich++;
			}else{
				/* read the transaction ID*/
				if(swich == 0){
					new->transation_ID[i++] = c;
				/* read the card ID */
				}else if(swich == 1){
					new->card_ID[i++] = c;
				/* read the transaction time*/
				}else if(swich == 2){
					time_recoder(c,&num, new, &newswich);
				}else if(swich == 3){
					int toint = c-'0';
					num = num*10+toint;
					new->amount= num;
				}	
			}
		}		
	}
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
	assert(list != NULL);
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
