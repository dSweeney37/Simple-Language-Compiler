/*
	Author: Daniel T. Sweeney
	Class: CSE340 - 93983
	Assignment: Project 3
	Date: 11/ /2020
	References: https://www.geeksforgeeks.org/unordered_map-in-cpp-stl/
*/

#include <iostream>
#include <unordered_map>
#include "compiler.h"
#include "lexer.h"




class Parser {
	public:
		InstructionNode* ParseProgram();
		std::unordered_map<std::string, int> locationMap;


	private:
		LexicalAnalyzer lexer;

		Token Expect(TokenType);
		int Location(std::string);
		void ParseVarSec();
		void ParseIdList();
		InstructionNode* ParseBody();
		InstructionNode* ParseStmtList();
		InstructionNode* ParseStmt();
		InstructionNode* ParseAssignStmt();
		void ParseExpr(InstructionNode*);
		int ParsePrimary();
		ArithmeticOperatorType ParseOp();
		InstructionNode* ParseOutputStmt();
		InstructionNode* ParseInputStmt();
		InstructionNode* ParseWhileStmt();
		InstructionNode* ParseIfStmt();
		void ParseCondition(InstructionNode*);
		ConditionalOperatorType ParseRelop();
		InstructionNode* ParseSwitchStmt();
		InstructionNode* ParseForStmt();
		void ParseCaseList(InstructionNode*, InstructionNode*);
		void ParseCase(InstructionNode*, InstructionNode*);
		InstructionNode* ParseDefaultCase();
		void ParseInptus();
		void ParseNumList();
		void SyntaxError();
};
