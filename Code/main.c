// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"



/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

/** 
 * Function to be written by students
 */
void computeExpressions(FILE* input);
Queue* shuntingYard(Queue* infix);
bool isSymbol(char c);
bool is_condition_correct_case_operator(const Token*oper1, const Token*oper2);
Queue* stringToTokenQueue(const char* expression);
/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}

bool is_condition_correct_case_operator(const Token*oper1, const Token*oper2){
	int o2_precedence = token_operator_priority(oper2);
	int o1_precedence = token_operator_priority(oper1);
	bool isNotLeftParenthesis = !(token_parenthesis(oper2) == '(');
	bool isO2GreaterO1 = o2_precedence>o1_precedence;
	bool isEqualPrecedence = (o2_precedence == o1_precedence);
	bool isO1LeftAssociative = token_operator_leftAssociative(oper1);
	bool firstCondition = isNotLeftParenthesis;
	bool secondCondition = isO2GreaterO1||(isEqualPrecedence&&isO1LeftAssociative);
	bool result = firstCondition&&secondCondition;
	return result;
}

Queue* shuntingYard(Queue* infix){
	Queue* postfix = create_queue();
	int stack_size = queue_size(infix);
	Stack* operatorStack = create_stack(stack_size);
	while (!queue_empty(infix)){
		const Token* t = queue_top(infix); //queue_top returns a generic ppointer
		if (token_is_number(t))
		{
			queue_push(postfix, t);
			queue_pop(infix);
			continue;
		}
		if (token_is_operator(t)){
			/*while (
			- firstCondition
            there is an operator o2 at the top of the operator stack which is not a left parenthesis, 
            AND 
			- secondCondition
			(o2 has greater precedence than o1 OR (o1 and o2 have the same precedence and o1 is left-associative))
			): 
				pop o2 from the operator stack into the output queue

			push o1 onto the operator stack
			*/
			
			if (stack_empty(operatorStack))
			{
				stack_push(operatorStack, t);
				queue_pop(infix);
				continue;
			}
			else{
				const Token* o2 = stack_top(operatorStack);
				
				bool is_condition_true = is_condition_correct_case_operator(t, o2);
				while (is_condition_true)
				{
					//pop o2 from the operator stack into the output queue
					queue_push(postfix, o2);
					stack_pop(operatorStack);
					o2 = stack_top(operatorStack);
					is_condition_true = is_condition_correct_case_operator(t, o2);
				}
				stack_push(operatorStack, t);
				queue_pop(infix);
				continue;
			}
			}
		if (token_is_parenthesis(t) && (token_parenthesis(t) == '(')){
			stack_push(operatorStack, t);
			queue_pop(infix);
			continue;
			}

		if (token_is_parenthesis(t) && (token_parenthesis(t) == ')')){
		
        //while the operator at the top of the operator stack is not a left parenthesis:
             //{assert the operator stack is not empty}
            /* If the stack runs out without finding a left parenthesis, then there are mismatched parentheses. */
                 //pop the operator from the operator stack into the output queue
        //{assert there is a left parenthesis at the top of the operator stack}
            //pop the left parenthesis from the operator stack and discard it
			while (!stack_empty(operatorStack)){
				const Token* operatorStack_top = stack_top(operatorStack);
				if ((token_is_operator(operatorStack_top))||(token_is_parenthesis(operatorStack_top)&&(token_parenthesis(operatorStack_top) != '('))){
					queue_push(postfix,operatorStack_top);
					stack_pop(operatorStack);
				}else{
					break;
				}
			}
			if (stack_empty(operatorStack))
			{
				
				fprintf(stderr, "There are a mismatched parenthesis! \n");
				exit(1);
			}
			const Token* operatorStack_top = stack_top(operatorStack);
			if(token_parenthesis(operatorStack_top) == '('){
				stack_pop(operatorStack);
				queue_pop(infix);
				continue;
			}else{
				
				fprintf(stderr, "mismatched\n");
				exit(2);
			}
			
		}	
		}
	while (!stack_empty(operatorStack)){
		if (token_is_parenthesis(stack_top(operatorStack))){
			fprintf(stderr, "There are a mismatched parenthesis! \n");
			exit(1);
		}
		queue_push(postfix,stack_top(operatorStack));
		stack_pop(operatorStack);
	}
	delete_stack(&operatorStack);
	return postfix;
}
	


Queue* stringToTokenQueue(const char* expression){
	//Queue contient le resultat
	Queue* q = create_queue();
	const char* curpos = expression;

	
	while(*curpos != '\0'){
		if(*curpos == ' ' || *curpos == '\n'){
			curpos +=1;
			continue;
		}

		if(isdigit(*curpos)){
			int nb_digit = 1;
			const char* curpos_copy = curpos + 1;
			// Calculate the digit number
			
			while (isdigit(*curpos_copy))
			{
				
				nb_digit += 1;
				curpos_copy += 1;
			}
			// Calculate the value:
			//atof return a double
			float v = (float) atof(curpos);
			//printf("v = %f\n", v);
			curpos+=nb_digit;
			Token *t = create_token_from_value(v);
			queue_push(q, t);
		}
		if (isSymbol(*curpos)){
			
			Token *t = create_token_from_string(curpos, 1);
			queue_push(q, t);
			curpos +=1;
		}	
	}
	
	return q;

}
void computeExpressions(FILE* input) {
	size_t buffer_size = 0;
	char* line = NULL;
	 
	// getline can automatically allocate memory 
	//size_t buffer_size = 512;
	
	//line = (char*) malloc(buffer_size*sizeof(char));
	while(getline(&line, &buffer_size, input) != -1){
		printf("Input: %s", line);
		printf("Infix: ");
		Queue *my_infix = stringToTokenQueue(line);
		print_queue(stdout, my_infix);
		printf("\n");
		Queue* my_postfix = shuntingYard(my_infix);
		
		printf("Postfix: ");
		print_queue(stdout, my_postfix);
		printf("\n\n");

		// free 
		ptrQueue* q = &my_infix;
		delete_queue(q);
		ptrQueue* q_post = &my_postfix;
		delete_queue(q_post);
	}
	free(line);
	return;
	
}
bool isSymbol(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || 
            c == '^' || c == '(' || c == ')');
}

void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t); // token_dump = print token to file f or output
}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}
