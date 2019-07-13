#include "baseExpression.h"

BaseExpression::~BaseExpression() { }

uint16_t ControlAttribute::id = 0;

void ControlAttribute::Reset()
{
	id = 0;
}
