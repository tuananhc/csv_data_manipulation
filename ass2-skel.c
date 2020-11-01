/* Program to evaluate candidate routines for Robotic Process Automation.

  Skeleton program written by Artem Polyvyanyy, artem.polyvyanyy@unimelb.edu.au,
  September 2020, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial skeleton
  file, the  program contained in this submission is completely my own
  individual work, except where explicitly noted by further comments that
  provide details otherwise.  I understand that work that has been developed
  by another student, or by me in collaboration with other students, or by
  non-students as a result of request, solicitation, or payment, may not be
  submitted for assessment in this subject.  I understand that submitting for
  assessment work developed by or in collaboration with other students or
  non-students constitutes Academic Misconduct, and may be penalized by mark
  deductions, or by other penalties determined via the University of
  Melbourne Academic Honesty Policy, as described at
  https://academicintegrity.unimelb.edu.au.

  (2) I also certify that I have not provided a copy of this work in either
  softcopy or hardcopy or any other form to any other student, and nor will I
  do so until after the marks are released. I understand that providing my
  work to other students, regardless of my intention or any undertakings made
  to me by that other student, is also Academic Misconduct.

  (3) I further understand that providing a copy of the assignment
  specification to any form of code authoring or assignment tutoring service,
  or drawing the attention of others to such services and code that may have
  been made available via such a service, may be regarded as Student General
  Misconduct (interfering with the teaching activities of the University
  and/or inciting others to commit Academic Misconduct).  I understand that
  an allegation of Student General Misconduct may arise regardless of whether
  or not I personally make use of such solutions or sought benefit from such
  actions.

   Signed by: [Tuan Anh Chau]
   Dated:     [October 16th 2020]

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

/* #define's ---------------------------------------------------------------*/

#define ASIZE         26
#define ENDINPUT      '#'
#define SEPERATOR     ':'
#define CH_NL         '\n'
#define CH_CR         '\r'
#define CH_NULL       '\0'
#define INITIAL       200 
#define ASCII_A       97
#define ASCII_Z       122
#define TRUEVAL       1
#define FALSEVAL      0
#define STAGE0        "==STAGE 0==============================="
#define STAGE1        "==STAGE 1==============================="
#define STAGE2        "==STAGE 2==============================="
#define END           "==THE END==============================="
#define STAGESEPERATE "----------------------------------------"

/* type definitions --------------------------------------------------------*/

// state (values of the 26 Boolean variables)
typedef unsigned char state_t[ASIZE];

// action
typedef struct action action_t;
struct action {
    char name;        // action name
    state_t precon;   // precondition
    state_t effect;   // effect
};

// step in a trace
typedef struct step step_t;

struct step {
    action_t *action; // pointer to an action performed at this step
    step_t   *next;   // pointer to the next step in this trace
};

// trace (implemented as a linked list)
typedef struct {
    step_t *head;     // pointer to the step in the head of the trace
    step_t *tail;     // pointer to the step in the tail of the trace
} trace_t;

/* function prototypes -----------------------------------------------------*/
trace_t* make_empty_trace(void);
trace_t* insert_at_tail(trace_t*, action_t*);
void free_trace(trace_t*);

/* my function prototypes --------------------------------------------------*/

// add your function prototypes here ...
void print_state(state_t state);
void initialize(action_t *action);
void alter_action(action_t *prev_act, action_t *post_act);
void assign_action(action_t *action, int command, int valid, int precon);
int check_valid(action_t *pre_act, action_t *post_act);
void print_act(action_t *action, int precon);
void initialize_step(step_t *step);
void alter_action(action_t *compare, action_t *action);
int check_candidate1(action_t *action1, action_t *action2);
int check_candidate2(action_t *action1, action_t *action2);
void copy_stat(action_t *action1, action_t *action2);
void stage0(action_t **action_traces, int num_acts, int num_steps, int valid);
void stage1_2(action_t **action_traces, action_t **actions, 
              int num_acts, int num_steps, int c, int stage);
int get_actions(action_t **actions, int c);
action_t* get_initial_action(int c);

/* where it all happens ----------------------------------------------------*/
int
main(int argc, char *argv[]) {
    int i, c = getchar();
    action_t *initial_action = get_initial_action(c);

    trace_t *trace;
    trace = make_empty_trace();
    trace = insert_at_tail(trace, initial_action);
    
    action_t **actions;
    /* the list of actions always have a size of at most 26 */
    actions = (action_t **)malloc(ASIZE * sizeof (action_t *));
    assert(actions);
    /* traverse to the next character */
    while (c != CH_NL) {
        c = getchar();
    }
    c = getchar();
    /* get the effects and precons of each actions */
    int num_acts = get_actions(actions, c);

    /* traverse to the next character */
    c = getchar();
    while(!isalpha(c) && c != EOF) {
        c = getchar();
    }
    
    int num_steps = 1, valid = 1;
    size_t cur_size = INITIAL;
    action_t **action_traces;
    action_traces = (action_t**)malloc(INITIAL*sizeof(action_t*));
    action_traces[0] = (action_t *)malloc(sizeof(action_t));
    assert(action_traces[0]);
    /* the first step is the initial action set above */
    action_traces[0] = initial_action;

    /* get the traces of the action sequence one by one*/
    while (c != ENDINPUT && c != EOF) {
        if (c == CH_CR || c == CH_NL) {
            c = getchar();
            continue;
        }
        /* reallocate the memory if the maximum 
        length of the sequence is reached */
        if (num_steps == cur_size) {
            cur_size *= 2;
            action_traces = realloc(action_traces, 
                                    cur_size * sizeof(action_t *));
            assert(action_traces);
        }
        for (i = 0; i <= num_acts && valid; i ++) {
            if (actions[i]->name == c) {
                /* check validity */
                valid = check_valid(action_traces[num_steps - 1], actions[i]);
                if (valid) {
                    action_traces[num_steps] = malloc(sizeof(action_t));
                    assert(action_traces[num_steps]);
                    /* copy the data from the previous step */
                    copy_stat(action_traces[num_steps - 1], 
                              action_traces[num_steps]);
                    action_traces[num_steps]->name = actions[i]->name;
                    /* change the value according 
                    to the effect of the action */
                    alter_action(actions[i], action_traces[num_steps]);
                    /* add to the trace */
                    trace = insert_at_tail(trace, action_traces[num_steps]);
                    break;
                } else {
                    /* if not valid the subsequent 
                    steps will be default to NULL */
                    break;
                }
            } 
        }
        num_steps++;
        c = getchar();
    }  

    stage0(action_traces, num_acts, num_steps, valid);
    if (c == ENDINPUT) {
        stage1_2(action_traces, actions, num_acts, num_steps, c, TRUEVAL);
    }
    if (c == ENDINPUT) {
        stage1_2(action_traces, actions, num_acts, num_steps, c, FALSEVAL);
    }
    /* free the memory allocated by malloc */
    for (i = 0; i <= num_acts; i ++) {
        free(actions[i]);
        actions[i] = NULL;
    }
    free(actions);
    actions = NULL;
    for (i = 0; i < num_steps; i ++) {
        free(action_traces[i]);
        action_traces[i] = NULL;
    }
    free(action_traces);
    action_traces = NULL;
    free_trace(trace);
    trace = NULL;
    return EXIT_SUCCESS;        // we are done !!! algorithms are fun!!!
}

/* function definitions -----------------------------------------------------*/

// Adapted version of the make_empty_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t
*make_empty_trace(void) {
    trace_t *R;
    R = (trace_t*)malloc(sizeof(*R));
    assert(R!=NULL);
    R->head = R->tail = NULL;
    return R;
}

// Adapted version of the insert_at_foot function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t 
*insert_at_tail(trace_t* R, action_t* addr) {
    step_t *new;
    new = (step_t*)malloc(sizeof(*new));
    assert(R!=NULL && new!=NULL);
    new->action = addr;
    new->next = NULL;
    if (R->tail==NULL) { /* this is the first insertion into the trace */
        R->head = R->tail = new; 
    } else {
        R->tail->next = new;
        R->tail = new;
    }
    return R;
}

// Adapted version of the free_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
void
free_trace(trace_t* R) {
    step_t *curr, *prev;
    assert(R!=NULL);
    curr = R->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(R);
}

/* my function definitions --------------------------*/

// add your function definitions here ...

/* print out the state */
void
print_state(state_t state) {
    int i;
    for (i = 0; i < ASIZE; i ++) {
        if (state[i] == 2) {
            printf("%d", 0);
        } else { 
            printf("%d", state[i]);
        }
    }
    printf("%c", CH_NL);
    return; 
}

/* set all values of the 2 states to 0 */
void 
initialize(action_t *action) {
    int i;
    for (i = 0; i < ASIZE; i ++) {
        action->precon[i] = 0;
        action->effect[i] = 0;
    }
    return;
}

/* change the action according to the effect
 of the action it being compared to*/
void
alter_action(action_t *compare, action_t *action) {
    int i;
    for (i = 0; i < ASIZE; i ++) {
        if (compare->effect[i] != 0) {
            action->effect[i] = compare->effect[i];
        }
    }
    return;
}

/* assign one of the 2 state of an action with a value */
void
assign_action(action_t *action, int command, int valid, int precon) {
    if (valid) {
        if (precon) {
            action->precon[command - ASCII_A] = 1;
        } else {
            action->effect[command - ASCII_A] = 1;
        }
    } else {
        /* set value to 2 instead of 0 to differentiate the 
        effect and the initial false value */
        if (precon) {
            action->precon[command - ASCII_A] = 2;
        } else {
            action->effect[command - ASCII_A] = 2;
        }
        /* the value 2 will be set to 0 once the state is printed */
    }
    return;
}

/* check if a previous action satisfies 
the precondition of the one after it */
int
check_valid(action_t *pre_act, action_t *post_act) {
    int i, valid = 1;
    for (i = 0; i < ASIZE; i ++) {
        if (post_act->precon[i] != 0) {
            if (post_act->precon[i] != pre_act->effect[i]) {
                /* if the value is 0 and the effect is changing 
                to false (saved as 2), the precondition is still satisfied */
                if (post_act->precon[i] == 2 && pre_act->effect[i] == 0) {
                    continue;
                } else {
                    valid = 0;
                    break;
                }
            }
        }
    }
    return valid;
}

/* check if a sequence output matches the conditions of the candidate exactly, 
used in stage 1 */
int
check_candidate1(action_t *action1, action_t *action2) {
    int i, valid = 1;
    for (i = 0; i < ASIZE; i ++) {
        if (action1->effect[i] != action2->effect[i]) {
            valid = 0;
            break;
        }
    }
    return valid;
}

/* check if a sequence output matches the states of the values in the 
candidate, used in stage 2 */
int
check_candidate2(action_t *action1, action_t *action2) {
    int i, valid = 1;
    for (i = 0; i < ASIZE; i ++) {
        if (action1->effect[i] != action2->effect[i]) {
            if (action1->effect[i] == 0 && action2->effect[i] == 2) {
                /* the desire output is still match, 
                as the conditions are both false */ 
                continue; 
            } else {
                valid = 0;
                break;
            }
        }
    }
    return valid;
}

/* copy the data of an action's precondition and effect to another one */
void 
copy_stat(action_t *action1, action_t *action2) {
    int i;
    for (i = 0; i < ASIZE; i ++) {
        action2->effect[i] = action1->effect[i];
        action2->precon[i] = action1->precon[i];
    }
    return;
}

/* print out the results acquired from stage 0 initialization */
void
stage0(action_t **action_traces, int num_acts, int num_steps, int valid) {
    int i;
    printf("%s%c", STAGE0, CH_NL);
    printf("Number of distinct actions: %d%c", num_acts + 1, CH_NL);
    printf("Length of the input trace: %d%c", num_steps - 1, CH_NL);
    printf("Trace status: ");
    if (valid) {
        printf("valid\n");
    } else {
        printf("invalid\n");
    }
    printf("%s%c", STAGESEPERATE, CH_NL);
    printf("  ");
    for (i = ASCII_A; i <= ASCII_Z; i ++) {
        printf("%c", i);
    }
    printf("%c", CH_NL);
    for (i = 0; i < num_steps; i ++) {
        if (action_traces[i]) {
            printf("%c ", action_traces[i]->name);
            print_state(action_traces[i]->effect);
        }
    }
    return;
}

/* print out the results acquired from stage 1 or 2 initialization */
void 
stage1_2(action_t **action_traces, action_t **actions, int num_acts, 
         int num_steps, int c, int stage1) {
    int found, i, j, k;
    action_t *sample, *candidate;
    sample = (action_t *)malloc(sizeof(action_t));
    assert(sample);
    initialize(sample);
    candidate = (action_t *)malloc(sizeof(action_t));
    assert(candidate);
    initialize(candidate);
    while (!isalpha(c)) {
        c = getchar();
    }
    if (stage1) {
        printf("%s%c", STAGE1, CH_NL);
    } else {
        printf("%s%c", STAGE2, CH_NL);
    }
    printf("Candidate routine: ");
    while (c != ENDINPUT && c != EOF) {
        if (c == CH_CR) {
            c = getchar();
            continue;
        } else if (c == CH_NL) {
            printf("%c", CH_NL);
            for (i = 1; i <= num_steps; i ++) {
                found = 0;
                for (j = i; j <= num_steps && !found && 
                     j != num_steps; j ++) {
                    for (k = 0; k <= num_acts; k ++) {
                        if (actions[k]->name == action_traces[j]->name) {
                            alter_action(actions[k], sample);
                        }
                    }
                    if (stage1) {
                        found = check_candidate1(candidate, sample);
                    } else {
                        found = check_candidate2(candidate, sample);
                    }
                }
                if (found) {
                    /* print out the result */
                    printf("%5d: ", i - 1);
                    for (; i < j; i ++) {
                        printf("%c", action_traces[i]->name);
                    }
                    i -= 1;
                    printf("%c", CH_NL);
                }
                initialize(sample);
            } /* set the values of sample back to all 0 and run again */
            initialize(sample);
            initialize(candidate);
            c = getchar();
            /* continue if the end of stage or 
            the end of file is not reached */
            if (c != ENDINPUT && c != EOF) {
                printf("%s%c", STAGESEPERATE, CH_NL);
                printf("Candidate routine: ");
            }
        } else {
            printf("%c", c);
            /* changes the value of the candidate
            according to the specified routine */
            for (i = 0; i <= num_acts; i ++) {
                if (actions[i]->name == c) {
                    alter_action(actions[i], candidate);
                }
            }
            c = getchar();
        }
        
    } 
    if (!stage1) {
        printf("%s%c", END, CH_NL);
    }
    /* free the memory allocated by malloc */
    free(candidate);
    candidate = NULL;
    free(sample);
    sample = NULL; 
}

int
get_actions(action_t **actions, int c) {
    int num_acts = 0, num_sprt = 0;
    /* the number of ':' will determine if the command is for assigning the
    precondition or the effect with true or false value */
    actions[num_acts] = (action_t *)malloc(sizeof(action_t));
    initialize(actions[num_acts]);
    while (c != EOF && c != ENDINPUT) {
        if (c == CH_CR) {
            c = getchar();
            continue;
        } else if (c == CH_NL) {
            /* end of line, reset number of seperator */
            num_sprt = 0;
            c = getchar();
            /* if the next character is  not '#' or EOF, 
            the process will continue */
            if (c != ENDINPUT && num_acts != ASIZE) {
                num_acts++;
                actions[num_acts] = (action_t *)malloc(sizeof(action_t));
                assert(actions[num_acts]);
                initialize(actions[num_acts]);
            }
            continue;
        } else if (c == SEPERATOR) {
            num_sprt ++;
            if (num_sprt == 2) {
                /* the name of the action is reached */
                c = getchar();
                actions[num_acts]->name = (char)c;
            }
            c = getchar();
            continue;
        } else {
            if (num_sprt == 0) {
                assign_action(actions[num_acts], c, TRUEVAL, TRUEVAL);
            } else if (num_sprt == 1) {
                assign_action(actions[num_acts], c, FALSEVAL, TRUEVAL);
            } else if (num_sprt == 3) {
                assign_action(actions[num_acts], c, TRUEVAL, FALSEVAL);
            } else if (num_sprt == 4) {
                assign_action(actions[num_acts], c, FALSEVAL, FALSEVAL);
            }
        }
        c = getchar();
    }
    return num_acts;
}

/* assign the initial true value into an action */
action_t
*get_initial_action(int c) {
    action_t *initial_action;
    initial_action = malloc(sizeof(action_t));
    assert(initial_action);
    initialize(initial_action);
    initial_action->name = '>';
    while (c != EOF && c != ENDINPUT) {
        if (c != CH_CR && c != CH_NL) {
            assign_action(initial_action, c, 1, 0);
        }
        c = getchar();
    }
    return initial_action;
}

/* Algorithms are fun */
/* ta-da-da-daa!!! ---------------------------------------------------------*/