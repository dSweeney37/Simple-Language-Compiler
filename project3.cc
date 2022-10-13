#include "project3.h"

using namespace std;




struct InstructionNode* parse_generate_intermediate_representation() {
	InstructionNode* n;
	Parser input;

	
	n = input.ParseProgram();
	return n;
}



// This function gets a token and checks if it is of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated this function is particularly useful to match
// terminals in a right hand side of a rule. Written by Mohsen Zohrevandi
// Note: Borrowed from Project 1
Token Parser::Expect(TokenType expected_type) {
	Token t = lexer.GetToken();


	if (t.token_type != expected_type) { SyntaxError(); }
	return t;
}



// Searches through "locationMap" for an entry that matches the parameter value and
// returns its "second" value (mem index) if one is found. Otherwise it returns
// the next available index in "mem" and increments the "next_available" variable.
int Parser::Location(string pVar) {
	for (auto var : locationMap) {
		if (var.first == pVar) { return var.second; }
	}

	return next_available++;
}



InstructionNode* Parser::ParseProgram() {
	// -> var_section & body & inputs
	InstructionNode* node;


	ParseVarSec();
	node = ParseBody();
	ParseInptus();
	Expect(END_OF_FILE);

	return node;
}



void Parser::ParseVarSec() {
	// -> id_list & SEMICOLON


	ParseIdList();
	
	// Verifies and consumes 'SEMICOLON'.
	Expect(SEMICOLON);
}



void Parser::ParseIdList() {
	// -> ID | ID & COMMA & id_list
	int pos;
	Token t;


	// Verifies and consumes 'ID'.
	t = Expect(ID);

	// Adds the variable and its index to "mem" to the "localeMap".
	pos = Location(t.lexeme);
	locationMap[t.lexeme] = pos;
	mem[pos] = 0;

	// Checks whether or not to continue parsing this block.
	t = lexer.GetToken();
	if (t.token_type == COMMA) { ParseIdList(); }
	else { lexer.UngetToken(1); }
}



InstructionNode* Parser::ParseBody() {
	// -> LBRACE & stmt_list & RBRACE
	InstructionNode* node;


	// Verifies and consumes "LBRACE".
	Expect(LBRACE);

	node = ParseStmtList();

	// Verifies and consumes "RBRACE".
	Expect(RBRACE);

	return node;
}



InstructionNode* Parser::ParseStmtList() {
	// -> stmt | stmt & stmt_list
	InstructionNode* node;
	InstructionNode* next;
	InstructionNode* ptr;
	Token p;

	
	node = ParseStmt();

	p = lexer.peek(1);
	if (p.token_type == ID || p.token_type == WHILE || p.token_type == IF || p.token_type == SWITCH || p.token_type == FOR
	 || p.token_type == OUTPUT || p.token_type == INPUT) {
		next = ParseStmtList();

		// Finds the last entry in the linked list of 'node' and then appends the 'next' node to it.
		ptr = node;
		while ((*ptr).next != NULL) { ptr = (*ptr).next; }
		(*ptr).next = next;
	}

	return node;
}



InstructionNode* Parser::ParseStmt() {
	// -> assign_stmt | while_stmt | if_stmt | switch_stmt | for_stmt | output_stmt | input_stmt
	InstructionNode* node = NULL;
	Token p = lexer.peek(1);


	if (p.token_type == ID) { node = ParseAssignStmt(); }
	else if (p.token_type == WHILE) { node = ParseWhileStmt(); }
	else if (p.token_type == IF) { node = ParseIfStmt(); }
	else if (p.token_type == SWITCH) { node = ParseSwitchStmt(); }
	else if (p.token_type == FOR) { node = ParseForStmt(); }
	else if (p.token_type == OUTPUT) { node = ParseOutputStmt(); }
	else if (p.token_type == INPUT) { node = ParseInputStmt(); }

	return node;
}



InstructionNode* Parser::ParseAssignStmt() {
	// -> ID & EQUAL & primary & SEMICOLON | ID & EQUAL & expr & SEMICOLON
	InstructionNode* node = new InstructionNode;
	Token p, t;


	// Verifies and consumes 'ID'.
	t = Expect(ID);
	(*node).type = ASSIGN;
	(*node).assign_inst.left_hand_side_index = Location(t.lexeme);
	(*node).next = NULL;

	// Verifies and consumes "EQUAL".
	Expect(EQUAL);

	p = lexer.peek(1);
	if (p.token_type == ID || p.token_type == NUM) {
		p = lexer.peek(2);

		if (p.token_type == PLUS || p.token_type == MINUS || p.token_type == MULT || p.token_type == DIV) {
			ParseExpr(node);
		}
		else {
			(*node).assign_inst.op = OPERATOR_NONE;
			(*node).assign_inst.operand1_index = ParsePrimary();
		}
	}
	else { SyntaxError(); }

	// Verifies and consumes 'SEMICOLON'.
	Expect(SEMICOLON);

	return node;
}



void Parser::ParseExpr(InstructionNode* pNode) {
	// -> primary & op & primary

	(*pNode).assign_inst.operand1_index = ParsePrimary();
	(*pNode).assign_inst.op = ParseOp();
	(*pNode).assign_inst.operand2_index = ParsePrimary();
}



int Parser::ParsePrimary() {
	// -> ID | NUM
	int pos;
	Token t;


	t = lexer.GetToken();
	pos = Location(t.lexeme);

	// If 't' is a number then add it to 'mem'.
	if (t.token_type == NUM) {
		locationMap[t.lexeme] = pos;
		mem[pos] = std::stoi(t.lexeme);
	}
	else if (t.token_type != ID) { SyntaxError(); }

	return pos;
}



ArithmeticOperatorType Parser::ParseOp() {
	// -> PLUS | MINUS | MULT | DIV
	Token t;


	t = lexer.GetToken();
	if (t.token_type == PLUS) { return OPERATOR_PLUS; }
	else if (t.token_type == MINUS) { return OPERATOR_MINUS; }
	else if (t.token_type == MULT) { return OPERATOR_MULT; }
	else if (t.token_type == DIV) { return OPERATOR_DIV; }
	else { SyntaxError(); }
}



InstructionNode* Parser::ParseOutputStmt() {
	// -> output & ID & SEMICOLON
	InstructionNode* node = new InstructionNode;
	Token t;


	// Verifies and consumes "OUTPUT".
	Expect(OUTPUT);
	// Verifies and consumes 'ID'.
	t = Expect(ID);
	
	(*node).type = OUT;
	(*node).output_inst.var_index = Location(t.lexeme);
	(*node).next = NULL;

	// Verifies and consumes 'SEMICOLON'.
	Expect(SEMICOLON);

	return node;
}



InstructionNode* Parser::ParseInputStmt() {
	// -> input & ID & SEMICOLON
	InstructionNode* node = new InstructionNode;
	Token t;


	// Verifies and consumes "INPUT".
	Expect(INPUT);
	// Verifies and consumes 'ID'.
	t = Expect(ID);
	
	(*node).type = IN;
	(*node).input_inst.var_index = Location(t.lexeme);
	(*node).next = NULL;

	// Verifies and consumes 'SEMICOLON'.
	Expect(SEMICOLON);

	return node;
}



InstructionNode* Parser::ParseWhileStmt() {
	// -> WHILE & condition & body
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* jmp = new InstructionNode;
	InstructionNode* ptr;


	// Loads the "CJMP", "JMP", and "NOOP" nodes with their corresponding data.
	(*node).type = CJMP;
	(*node).cjmp_inst.target = noop;
	(*jmp).type = JMP;
	(*jmp).jmp_inst.target = node;
	(*jmp).next = noop;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	// Verifies and consumes "WHILE".
	Expect(WHILE);
	ParseCondition(node);
	(*node).next = ParseBody();

	// Finds the last entry in the linked list of 'node' and then appends the 'jmp' node to it.
	ptr = node;
	while ((*ptr).next != NULL) { ptr = (*ptr).next; }
	(*ptr).next = jmp;

	return node;
}



InstructionNode* Parser::ParseIfStmt() {
	// -> IF & condition & body
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* ptr;


	// Verifies and consumes "IF".
	Expect(IF);
	(*node).type = CJMP;
	(*node).cjmp_inst.target = noop;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	ParseCondition(node);
	(*node).next = ParseBody();

	// Finds the last entry in the linked list of 'node' and then appends the 'noop' node to it.
	ptr = node;
	while ((*ptr).next != NULL) { ptr = (*ptr).next; }
	(*ptr).next = noop;

	return node;
}



void Parser::ParseCondition(InstructionNode* pNode) {
	// -> primary & relop & primary

	(*pNode).cjmp_inst.operand1_index = ParsePrimary();
	(*pNode).cjmp_inst.condition_op = ParseRelop();
	(*pNode).cjmp_inst.operand2_index = ParsePrimary();
}



ConditionalOperatorType Parser::ParseRelop() {
	// -> GREATER | LESS | NOTEQUAL
	Token t;


	// Verifies and consumes "GREATER" or "LESS" or "NOTEQUAL".
	t = lexer.GetToken();
	if (t.token_type != GREATER && t.token_type != LESS && t.token_type != NOTEQUAL) { SyntaxError(); }

	if (t.token_type == GREATER) { return CONDITION_GREATER; }
	else if (t.token_type == LESS) { return CONDITION_LESS; }
	else if (t.token_type == NOTEQUAL) { return CONDITION_NOTEQUAL; }
}



InstructionNode* Parser::ParseSwitchStmt() {
	// -> SWITCH & ID & LBRACE & case_list & RBRACE | SWITCH & ID & LBRACE & case_list & default_case & RBRACE
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* ptr;
	Token p, t;


	// Verifies and consumes 'SWITCH'.
	Expect(SWITCH);
	// Verifies and consumes 'ID'.
	t = Expect(ID);

	// Loads 'node' with all of the information used by each case in the switch statement.
	(*node).type = CJMP;
	(*node).cjmp_inst.operand1_index = Location(t.lexeme);;
	(*node).cjmp_inst.condition_op = CONDITION_NOTEQUAL;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	// Verifies and consumes "LBRACE".
	Expect(LBRACE);

	ParseCaseList(node, noop);

	// Finds the last entry in the linked list of 'node' and then appends the 'noop' node to it.
	ptr = node;
	while ((*ptr).next != NULL) { ptr = (*ptr).next; }

	p = lexer.peek(1);
	if (p.token_type == DEFAULT) {
		(*ptr).next = ParseDefaultCase();

		ptr = (*ptr).next;
		while ((*ptr).next != NULL) { ptr = (*ptr).next; }
	}
	
	(*ptr).next = noop;

	// Verifies and consumes "RBRACE".
	Expect(RBRACE);

	return node;
}



InstructionNode* Parser::ParseForStmt() {
	// -> FOR & LPAREN & assign_stmt & condition & SEMICOLON & assign_stmt & RPAREN & body
	InstructionNode* body;
	InstructionNode* calc;
	InstructionNode* comp = new InstructionNode;
	InstructionNode* jmp = new InstructionNode;
	InstructionNode* node;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* ptr;


	// Verifies and consumes 'FOR'.
	Expect(FOR);
	// Verifies and consumes 'LPAREN'.
	Expect(LPAREN);

	node = ParseAssignStmt();
	ParseCondition(comp);

	// Verifies and consumes 'SEMICOLON'.
	Expect(SEMICOLON);

	calc = ParseAssignStmt();

	// Verifies and consumes 'RPAREN'.
	Expect(RPAREN);
	
	body = ParseBody();

	// Loads the 'calc', 'comp', 'jmp', 'node', and 'noop' nodes with their corresponding data.
	(*calc).next = jmp;
	(*comp).type = CJMP;
	(*comp).next = body;
	(*comp).cjmp_inst.target = noop;
	(*jmp).type = JMP;
	(*jmp).jmp_inst.target = comp;
	(*jmp).next = noop;
	(*node).next = comp;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	// Finds the last entry in the linked list of the body and then appends the 'jmp' node to it.
	ptr = (*comp).next;
	while ((*ptr).next != NULL) { ptr = (*ptr).next; }
	(*ptr).next = calc;

	return node;
}



void Parser::ParseCaseList(InstructionNode* pNode, InstructionNode* pNoop) {
	// -> case | case & case_list
	Token p;


	ParseCase(pNode, pNoop);

	// Checks whether or not to continue parsing this block.
	p = lexer.peek(1);
	if (p.token_type == CASE) {
		InstructionNode* node = new InstructionNode;


		(*pNode).next = node;

		(*node).type = CJMP;
		(*node).cjmp_inst.operand1_index = (*pNode).cjmp_inst.operand1_index;
		(*node).cjmp_inst.condition_op = CONDITION_NOTEQUAL;

		ParseCaseList(node, pNoop);
	}
	else { (*pNode).next = NULL; }
}



void Parser::ParseCase(InstructionNode* pNode, InstructionNode* pNoop) {
	// -> CASE & NUM & COLON & body
	InstructionNode* ptr;
	int pos;
	Token t;


	// Verifies and consumes "CASE".
	Expect(CASE);
	
	// Verifies and consumes "NUM".
	t = Expect(NUM);
	pos = Location(t.lexeme);
	locationMap[t.lexeme] = pos;
	mem[pos] = std::stoi(t.lexeme);

	// Sets the second operand for the comparison.
	(*pNode).cjmp_inst.operand2_index = pos;

	// Verifies and consumes "COLON".
	Expect(COLON);

	// If operand1 is not less or greater than operand2, then they are equal which means the program will
	// will jump to '.target' rather than next so we simply use '.target' for the 'next' instruction and
	// next fpr the 'jump-to' instruction.
	(*pNode).cjmp_inst.target = ParseBody();

	// Finds the last node in the body and then assigns "noop" to its next value.
	ptr = (*pNode).cjmp_inst.target;
	while ((*ptr).next != NULL) { ptr = (*ptr).next; }
	(*ptr).next = pNoop;
}



InstructionNode* Parser::ParseDefaultCase() {
	// -> DEFAULT & COLON & body
	InstructionNode* node;


	// Verifies and consumes "DEFAULT".
	Expect(DEFAULT);
	Expect(COLON);
	node = ParseBody();

	return node;
}



void Parser::ParseInptus() {
	// -> num_list

	ParseNumList();
}



void Parser::ParseNumList() {
	// -> NUM | NUM num_list
	Token p, t;


	// Verifies and consumes "NUM".
	t = Expect(NUM);

	// Appends the input to the "input" vector.
	inputs.push_back(std::stoi(t.lexeme));

	// Checks whether or not to continue parsing this block.
	p = lexer.peek(1);
	if (p.token_type == NUM) { ParseNumList(); }
}



void Parser::SyntaxError() {
	cout << "SYNTAX ERROR !!!" << endl;
	exit(1);
}
