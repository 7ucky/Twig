/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#include "parser.hpp"

#include <QVariant>
#include <assert.h>

#include "../nuria/templateengine.hpp"
#include "templateengine_p.hpp"
#include <nuria/tokenizer.hpp>
#include "../grammar/twig.h"
#include <nuria/debug.hpp>
#include "astnodes.hpp"

namespace Nuria {
namespace Template {

class ParserPrivate {
public:
	
	Node *node = nullptr;
	TemplateEngine *engine = nullptr;
	TemplateError error;
	TemplateProgramPrivate *curProg = nullptr;
	
};

static constexpr inline Location toLoc (const Token *tok) {
	return Location (tok->row, tok->column);
}

static void addTrim (ParserPrivate *dptr, Template::Node *node,
                     const Token *left, const Token *right,
                     const Token *innerLeft = nullptr, const Token *innerRight = nullptr) {
	
	int mode = Trim::None;
	if (left && left->tokenId == TOK_TRIM) mode |= Trim::Left;
	if (right && right->tokenId == TOK_TRIM) mode |= Trim::Right;
	if (innerLeft && innerLeft->tokenId == TOK_TRIM) mode |= Trim::InnerLeft;
	if (innerRight && innerRight->tokenId == TOK_TRIM) mode |= Trim::InnerRight;
	
	if (mode != Trim::None && node) {
		dptr->curProg->info->trim.insert (node, mode);
	}
	
}

static void addTrim (ParserPrivate *dptr, Node *node, bool left, bool right) {
	int mode = Trim::None;
	if (left) mode |= Trim::Left;
	if (right) mode |= Trim::Right;
	
	if (mode != Trim::None && node) {
		dptr->curProg->info->trim.insert (node, mode);
	}
	
}

static ExpressionNode *maybeNegate (const Token *isNot, ExpressionNode *expr) {
	if (isNot->tokenId == TOK_NOT) {
		return new ExpressionNode (expr->loc, expr, Operator::Not, nullptr);
	}
	
	return expr;
}

// Hide symbols generated by LEMON
namespace {

#undef NDEBUG
// Output of LEMON
#include "../grammar/twig.c"

}

}
}

Nuria::Template::Parser::Parser (TemplateEngine *engine)
        : QObject (engine), d_ptr (new ParserPrivate)
{
	
	this->d_ptr->engine = engine;
	
}

Nuria::Template::Parser::~Parser () {
	delete this->d_ptr->node;
	delete this->d_ptr;
}

bool Nuria::Template::Parser::parse (const QVector< Token > &tokens, TemplateProgramPrivate *dptr) {
//	TwigParserTrace (stdout, "Twig> "); // Uncomment for parser-level traces
	
	clear ();
	this->d_ptr->curProg = dptr;
	
	// Create parser
	void *parser = TwigParserAlloc (&malloc);
	
	// Parse!
	for (int i = 0; i < tokens.length (); i++) {
		const Token &tok = tokens.at (i);
		TwigParser (parser, tok.tokenId, &tok, this->d_ptr);
	}
	
	// End of stream.
	TwigParser (parser, 0, nullptr, this->d_ptr);
	
	// Destroy parser
	TwigParserFree (parser, &free);
	
	// 
	this->d_ptr->curProg = nullptr;
	return !this->d_ptr->error.hasFailed ();
}

Nuria::Template::Node *Nuria::Template::Parser::baseNode () {
	return this->d_ptr->node;
}

Nuria::Template::Node *Nuria::Template::Parser::stealBaseNode () {
	Template::Node *node = this->d_ptr->node;
	this->d_ptr->node = nullptr;
	return node;
}

void Nuria::Template::Parser::clear () {
	delete this->d_ptr->node;
	this->d_ptr->node = nullptr;
	this->d_ptr->error = TemplateError ();
}

Nuria::TemplateError Nuria::Template::Parser::lastError () const {
	return this->d_ptr->error;
}
