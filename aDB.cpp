#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <vector>
#include <map>
#include <iostream>

/*
Authors: 
	 
Last Edited: Nov. 10, 2016

*/

#define MAXWORD 50 //maximum word size

using namespace std;

typedef enum{
    tokSELECT,tokFROM,tokWHERE,tokIS,tokNOT,tokBETWEEN,tokAND,tokINSERT,
    tokINTO,tokVALUES,tokDELETE, //reserved words
    tokADD,tokSUB,tokMULT,tokDIV, //arithmetic operators
    tokGRTR,tokLESS,tokEQL,tokGEQ,tokLEQ,tokNOTEQ, //comparison operators
    tokLPAR,tokRPAR,tokCOMMA,tokQUOTE, //delimeter
    tokNAME,
    NAtk,
    tokEND,tokTEXT,tokLET,tokVAL,tokEXPUNC,tokSEMICOL,
    tokERR

}TokenType;

typedef struct tokenTag{
    char str[MAXWORD];
    TokenType type;
    int pos; //line number
}Token;

typedef enum{
    selectNode,attrNode,whereNode,tblNode,stmtNode,exprNode,op1Node,op2Node,op3Node,
    nameNode,insertNode,tupleNode
}NodeType;

typedef struct nodeTag{
    NodeType type;
    Token token;
    struct nodeTag *child1;
    struct nodeTag *child2;
    struct nodeTag *child3;
}PNode;

char arithOperator[]={'*','/','+','-'};
char delimeter[]={'(',')',',','"','=',';'};
map<string,TokenType> reservedWord;
string code;
int offset=0,lineNum=0,tokcount=0;
Token *toks=NULL;
Token tk = {"N/A",NAtk,0};

PNode* op1();
PNode* op2();
PNode* op3();
PNode* loop();
PNode* expr();

void make_tokenmap(){
    reservedWord["SELECT"]=tokSELECT;
    reservedWord["FROM"]=tokFROM;
    reservedWord["WHERE"]=tokWHERE;
    reservedWord["IS"]=tokIS;
    reservedWord["NOT"]=tokNOT;
    reservedWord["BETWEEN"]=tokBETWEEN;
    reservedWord["AND"]=tokAND;
    reservedWord["INSERT"]=tokINSERT;
    reservedWord["INTO"]=tokINTO;
    reservedWord["VALUES"]=tokVALUES;
    reservedWord["DELETE"]=tokDELETE;

}

int isReservedChar(char c){
    if (c=='(' || c==')' || c==',' || c=='!' || c=='"' || c=='+' || c=='-' || 
	c=='*' || c=='/' || c=='%' || c=='=' || c=='<' || c=='>'){
	return 1;
    }else return 0;
}

int isDelimeter(char c){
    if (c=='(' || c==')' || c==',' || c=='"' || c==';'){
	return 1;
    }else return 0;
}

TokenType getDelimeterType(char c){
    if(c=='(') return tokLPAR;
    if(c==')') return tokRPAR;
    if(c==',') return tokCOMMA;
    if(c=='"') return tokQUOTE;
    if(c==';') return tokSEMICOL;
}

int isArithOperator(char c){  //char parameter
    if (c=='+' || c=='-' || c=='*' || c=='/' || c=='%')
	return 1;
    else return 0;
}

int isArithmeticOperator(string c){ //string parameter
    if (c=="+" || c=="-" || c=="*" || c=="/" || c=="%")
    	return 1;
    else return 0;
}

int isCompOperator(char c){
    if(c=='>' || c=='<' || c=='=' || c=='!')
	return 1;
    else return 0;
}

int isComparisonOperator(string c){
    if(c==">" || c=="<" || c=="=" || c==">=" || c=="<=" || c=="!=" || c=="<>")
	return 1;
    else return 0;
}

TokenType getArithType(char c){
    if(c=='+') return tokADD;
    if(c=='-') return tokSUB;
    if(c=='*') return tokMULT;
    if(c=='/') return tokDIV;
}

TokenType getCompOpType(string c){
   if(c==">") return tokGRTR;
   if(c=="<") return tokLESS;
   if(c=="=") return tokEQL;
   if(c==">=") return tokGEQ;
   if(c=="<=") return tokLEQ;
   if(c=="!=") return tokNOTEQ;
   if(c=="<>") return tokNOTEQ;
   return tokERR;
}

int isStmt(TokenType type){
    if(type==tokSELECT || type==tokINSERT || type==tokDELETE)
	return 1;
    else return 0;
}

int isCompOp(TokenType type){
    if(type==tokGRTR || type==tokLESS || type==tokEQL || type==tokGEQ || type==tokLEQ || type==tokNOTEQ)
	return 1;
    else return 0;
}

int isRelOp(TokenType type){
   if(type==tokIS || type==tokBETWEEN || type==tokNOT)
	return 1;
   else
	return 0;
}

int scan_query(string linestream,int &lineNum){ //check for invalid characters
    int wordsize=0;
    char word[MAXWORD];
    for(int i=0;i<linestream.size();i++){
	if(linestream[i]=='\n'){ 
	    lineNum++;
	    continue;
	}

	/*if(linestream[i]=='-'){ 
	    if(linestream[i+1]=='-'){ //ignore comments
		lineNum++;
		continue;
	    }
	}*/

    	if(isalnum(linestream[i])){
	    word[wordsize++]=linestream[i];
	    if(wordsize==MAXWORD){
		  printf("ERROR: A word in line %d is too long. \n",lineNum);
		  return 1;
	    }
	}else if(isspace(linestream[i]) || isReservedChar(linestream[i])){
	    wordsize = 0;
	}else if(ispunct(linestream[i])){
	    continue;
	}else{
	    printf("Error: Invalid character '%c' in line %d. \n",linestream[i],lineNum);
	    return 1;
	}

	
    }
}

Token anlzr(){
    int idx=0;
    Token token; 
    string temp; 
    for(int i=offset;i<code.size();i++){
	if(code[i]=='\n'){
	    lineNum++;
	    continue; 
	}

     	if(code[i]=='"'){ 
	    idx=i+1; //do not include "
	    while(i<code.size() && code[++i]!='"'){} //CONCAT print's text
	    temp=code.substr(idx,i-idx).c_str();
	    strcpy(token.str, temp.c_str());
	    token.pos = lineNum;
	    token.type = tokTEXT; 
	    offset = i+1;
	    return token;
	}else if(code[i]=='\''){
	    idx=i+1; //do not include "
	    while(i<code.size() && code[++i]!='\''){} //CONCAT print's text
	    temp=code.substr(idx,i-idx).c_str();
	    strcpy(token.str, temp.c_str());
	    token.pos = lineNum;
	    token.type = tokTEXT;
	    offset = i+1;
	    return token;
	}
        
        if(isalpha(code[i])){ //save keywords/attributes
	    idx=i;
	    while(isalnum(code[i]) || code[i]=='_'){code[i]=toupper(code[i++]);}
	    temp=code.substr(idx,i-idx).c_str();
            strcpy(token.str,temp.c_str());
	    token.pos = lineNum;
            if(reservedWord.find(temp)!=reservedWord.end()){ 
		  token.type = reservedWord.find(temp)->second;
	    }else{
		  token.type = tokNAME; //identifier
	    }
	    //printf("%s\n",temp.c_str());
	    offset=i; 
	    return token;
	}else if(isdigit(code[i])){
	    idx=i;
            int dotcount=0,maxdecplace=0;
  	    while(isdigit(code[i]) || code[i]=='.'){
		i++; 
		if(code[i]=='.'){
		    dotcount++;
		    maxdecplace=1;
		}
		if(maxdecplace) maxdecplace++;
	    }
	    if(code[i-1]=='.' || dotcount>1){
		printf("ERROR: Invalid numerical value in line %d.\n",lineNum);
		token.type = tokEND;
	    }
	    if(maxdecplace>5){
		printf("ERROR: The decimal places of a number in line %d exceeds the limit(4).\n",lineNum);
		token.type = tokEND;
	    }
	    temp=code.substr(idx,i-idx).c_str();
	    strcpy(token.str, temp.c_str());
	    token.pos = lineNum;
	    token.type = tokVAL;
	    //printf("%s\n",temp.c_str());
	    offset = i--;
	    return token;
	}else if(ispunct(code[i])){
    	    token.str[0]=code[i];
    	    token.str[1]='\0'; 
	    if(isDelimeter(code[i])){
    		token.pos = lineNum;
    		token.type = getDelimeterType(code[i]);
	    }else if(isArithOperator(code[i])){
    		token.pos = lineNum; 
    		token.type = getArithType(code[i]); 
	    }else if(isCompOperator(code[i])){
		//check 2nd char
		if(isCompOperator(code[i+1])){
		    token.str[1]=code[++i]; //add another character
		    token.str[2]='\0'; 
		    token.pos = lineNum;
		    temp=code.substr(i-1,2);
		    token.type = getCompOpType(temp);
		}else{
		    temp=code.substr(i,1);
		    token.type = getCompOpType(temp);
		}
	    }else{ 
    		token.pos = lineNum;
    		token.type = tokEXPUNC;
	    }
	    //printf("%s\n",token.str);
	    offset = ++i;
	    return token;
	}
    }
    token.type = tokEND; //duplicates the last token if not ';'
    return token;
}

PNode* createNode(NodeType type){
    PNode *ptreenode = (PNode*) malloc(sizeof(PNode));
    ptreenode->type = type;
    //ptreenode->token;
    ptreenode->child1 =ptreenode->child2 =ptreenode->child3 =NULL;
    return ptreenode;
}

PNode* attr_more(){
    PNode *ptreenode = createNode(attrNode);
    if(tk.type == tokNAME){
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
	if(tk.type==tokCOMMA){
	    tk = toks[++offset];
 	    ptreenode->child1 = attr_more(); //more attributes in the project operation		
	}
    }else{
	printf("ERROR: Expecting another attribute name in line %d.\n",tk.pos);
	exit(1);
    }
    return ptreenode;
}

PNode* attrib(){
    PNode *ptreenode = createNode(attrNode);
    if(tk.type == tokNAME){
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
	if(tk.type==tokCOMMA){
	    tk = toks[++offset]; 
 	    ptreenode->child1 = attr_more();
	}
    }else if(tk.type == tokMULT){ //* or all attributes
	tk = toks[++offset];
    }else{
	printf("ERROR: Invalid attribute name/s in the statement. %s\n",tk.str);
	exit(1);
    }
    return ptreenode;     
}

PNode* tble(){
    PNode *ptreenode = createNode(tblNode);
    if(tk.type == tokNAME){
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
	if(tk.type==tokCOMMA){
	    tk = toks[++offset];
	    ptreenode->child1 = tble(); //more tables	
	}
    }else{
	printf("ERROR: Invalid table name/s in the statement. %s\n",tk.str);
	exit(1);
    }
    return ptreenode;
}

PNode* op3(){
    PNode *ptreenode = createNode(op3Node);

    if(tk.type==tokLPAR){ 
	ptreenode->token = toks[offset]; //attach ( to syntax tree
        tk = toks[++offset]; 
        ptreenode->child1 = expr(); 
    	if(tk.type==tokRPAR){
            tk = toks[++offset];
            return ptreenode;
    	}else{
            printf("ERROR: Line %d expects ')'.\n",tk.pos);
            exit(1);
    	}
    }else if(tk.type==tokNAME){
	ptreenode->token = toks[offset]; //attach to syntax tree
        tk = toks[++offset]; 
        return ptreenode;
    }else if(tk.type==tokVAL){ 
	ptreenode->token = toks[offset]; //attach VAL to syntax tree
        tk = toks[++offset]; 
        return ptreenode;
    }else{
        printf("ERROR: Invalid expression.\n",tk.pos);
        exit(1);
    }
}

PNode* op2(){
    PNode *ptreenode = createNode(op2Node); 
    if(tk.type==tokSUB){ //negation
	ptreenode->token = toks[offset]; //attach NEG to syntax tree
        tk = toks[++offset];
        ptreenode->child1 = op2();
        return ptreenode;
    }else{
     	ptreenode->child1 = op3();
	return ptreenode;
    }
}

PNode* op1(){
    PNode *ptreenode = createNode(op1Node); 
    ptreenode->child1 = op2(); 
    if(tk.type==tokADD){ 
	ptreenode->token = toks[offset]; //attach ADD to syntax tree
        tk = toks[++offset];
        ptreenode->child2 = op1();
        return ptreenode;
    }else if(tk.type==tokSUB){
	ptreenode->token = toks[offset]; //attach SUB to syntax tree
        tk = toks[++offset];
        ptreenode->child2 = op1();
        return ptreenode;
    }else{ 
        return ptreenode; //empty after op2
    }
}

PNode* expr(){
    PNode *ptreenode = createNode(exprNode); 
    ptreenode->child1 = op1();
    if(tk.type==tokMULT){
        ptreenode->token = toks[offset]; //attach MULT to syntax tree
        tk = toks[++offset];
        ptreenode->child2 = expr();
        return ptreenode;
    }else if(tk.type==tokDIV){
	ptreenode->token = toks[offset]; //attach DIV to syntax tree
        tk = toks[++offset];
        ptreenode->child2 = expr();
        return ptreenode;
    }else{
        return ptreenode; //empty after op1
    }
}

PNode* stmtment(){
    PNode *ptreenode = createNode(stmtNode);
    if(tk.type == tokNAME){
	ptreenode->token = toks[offset]; //attach MULT to syntax tree
	tk = toks[++offset];
	if(isCompOp(tk.type)){
	    tk = toks[++offset];
	    ptreenode->child1 = expr();
    	}else if(isRelOp(tk.type)){
	    tk = toks[++offset];
	    //Add Function
	}else{
	    printf("ERROR: Invalid where clause.");
	    exit(1);
	}
    }else{
	printf("ERROR: Invalid where clause.");
	exit(1);
    }
    return ptreenode;
}

PNode* wherecond(){
    PNode *ptreenode = createNode(whereNode);
    if(tk.type == tokWHERE){
	tk = toks[++offset];
	ptreenode->child1 = stmtment();
    }
    return ptreenode;
}

PNode* select_sql(){
    PNode *ptreenode = createNode(selectNode);
    ptreenode->child1 = attrib();
    
    if(tk.type==tokFROM){ 
	   tk = toks[++offset];
    }else{
       	printf("ERROR: Invalid select statement. %s\n",tk.str);
	exit(1);
    }

    ptreenode->child2 = tble();
    ptreenode->child3 = wherecond();

    if(tk.type==tokSEMICOL){ //end query
	    tk = toks[++offset];
	return ptreenode;
    }else{
   	    printf("ERROR: Invalid closing of the statement.\n");
	    exit(1);
    }
}

PNode* tplevalues(){
    PNode *ptreenode = createNode(tupleNode);
    if(tk.type==tokVAL || tk.type==tokTEXT){
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
	if(tk.type==tokCOMMA){
	    tk = toks[++offset];
	    ptreenode->child1 = tplevalues();
	}
    }else{
	printf("ERROR: Invalid insert statement: missing values.\n");
	exit(1);
    } 
    return ptreenode; 
}

PNode* tple(){
    PNode *ptreenode = createNode(tupleNode);
    if(tk.type==tokLPAR){ 
	tk = toks[++offset];
    }else{
	printf("ERROR: Invalid insert statement: expecting '(' after a comma.\n");
	exit(1);
    }
    ptreenode->child1 = tplevalues();

    if(tk.type==tokRPAR){ 
	tk = toks[++offset];
    }else{
	printf("ERROR: Invalid insert statement: no closing parenthesis.\n");
	exit(1);
    }

    if(tk.type==tokCOMMA){
	tk = toks[++offset];
	ptreenode->child2 = tple();
    }
    
    return ptreenode; 
}

PNode* attrib2(){
    PNode *ptreenode = createNode(attrNode);
    if(tk.type == tokNAME){
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
	if(tk.type==tokCOMMA){
	    tk = toks[++offset];
 	    ptreenode->child1 = attr_more();
	}
    }else{
	printf("ERROR: Invalid attribute name/s in the statement. %s\n",tk.str);
	exit(1);
    }

    if(tk.type==tokRPAR){ 
	tk = toks[++offset];
    }else{
	printf("ERROR: Invalid insert statement: no parenthesis after attribute name/s.\n");
	exit(1);
    }
    return ptreenode;  

}

PNode* tble2(){
   PNode *ptreenode = createNode(tblNode);
   if(tk.type==tokNAME){ 
	ptreenode->token = toks[offset]; //attach to syntax tree
	tk = toks[++offset];
   }else{
	printf("ERROR: Invalid insert statement: no table name.\n");
	exit(1);
   }
   return ptreenode;
}

PNode* insert_sql(){
    PNode *ptreenode = createNode(insertNode); 
    if(tk.type==tokINTO){ 
	   tk = toks[++offset];
    }else{
       	printf("ERROR: Invalid insert statement: no INTO keyword.\n");
	exit(1);
    }
    
    ptreenode->child1 = tble2();
  
    if(tk.type==tokLPAR){ 
	tk = toks[++offset];
	ptreenode->child2 = attrib2();
    }else{
	return ptreenode; //attributes are not specified
    }
    
    if(tk.type==tokVALUES){ //check reserved token "VALUES"
	tk = toks[++offset];
    }else{
       	printf("ERROR: Invalid insert statement: no VALUES keyword.\n");
	exit(1);
    }

    ptreenode->child3 = tple();

    if(tk.type==tokSEMICOL){ //end query
	    tk = toks[++offset];
    }else{
   	    printf("ERROR: Invalid closing of the statement.\n");
	    exit(1);
    }
    
    return ptreenode;
}

PNode* delete_sql(){

}

void execute_insert(PNode* root){
    string tblname = root->child1->token.str;
    //loadtree(tblname);
     

}

void parser(){
    tk = toks[0];
    PNode *root = NULL; 
    //start query
    if(tk.type==tokSELECT){ 
	tk = toks[++offset];
	root = select_sql();
    }else if(tk.type==tokINSERT){
    	tk = toks[++offset];
	root = insert_sql();
	execute_insert(root);
    }else if(tk.type==tokDELETE){
     	tk = toks[++offset];
	root = delete_sql();
    }else{
	printf("ERROR: Invalid start of the statement. %s\n",tk.str);
	exit(1);
    }

    if(tk.type==tokEND) //tk is global so tk will be the end node if parsing works fine
	    printf("Successful parsing!\n");
    else{
	    exit(1);
    }
}

int main(){
    string linestream;

    ifstream fp1("query.txt");
    if(!fp1.is_open()){
	    printf("ERROR: Cannot open file.");
	return 1;
    }

    make_tokenmap();

    for(int i=0;getline(fp1,linestream);i++){
	if(scan_query(linestream,lineNum)){
	    printf("Cannot run the program. \n");	    
	}
	code += linestream + "\n";
    }

    toks = (Token*) malloc(tokcount*sizeof(Token));
    lineNum=1;
    
    //GET tokens
    do{
	    tokcount++;
	    toks = (Token*)realloc(toks,tokcount*sizeof(Token)); 
	    toks[tokcount-1]= anlzr(); 
	    printf("%s\t%d\n",toks[tokcount-1].str, toks[tokcount-1].type);
    }while(toks[tokcount-1].type != tokEND);

    offset=0;
    lineNum=0;
    parser();
}
