/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2016 - Raphael Araújo e Silva <raphael@pgmodeler.com.br>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "operatorclasselement.h"

OperatorClassElement::OperatorClassElement(void)
{
	element_type=OPERATOR_ELEM;
	function=nullptr;
	_operator=nullptr;
	strategy_number=0;
	op_family=nullptr;
}

void OperatorClassElement::setFunction(Function *func, unsigned stg_number)
{
	//Raises an exception case the function is not allocated
	if(!func)
		throw Exception(ERR_ASG_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	//Raises an error case the stratagy number is zero (invalid)
	else if(stg_number==0)
		throw Exception(ERR_ASG_INV_SUPSTG_NUMBER,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	//Clear the attributes not related to the FUNCTION element type
	this->_operator=nullptr;
	this->op_family=nullptr;

	//Configure the attributes that belongs to the element type
	this->function=func;
	this->strategy_number=stg_number;
	this->element_type=FUNCTION_ELEM;
}

void OperatorClassElement::setOperator(Operator *oper, unsigned stg_number)
{
	//Raises an error in case the operator is not allocated
	if(!oper)
		throw Exception(ERR_ASG_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	//Raises an error case the stratagy number is zero (invalid)
	else if(stg_number==0)
		throw Exception(ERR_ASG_INV_SUPSTG_NUMBER,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	//Clear the attributes not related to the OPERATOR element type
	this->function=nullptr;

	//Configure the attributes that belongs to the element type
	this->_operator=oper;
	this->strategy_number=stg_number;
	this->element_type=OPERATOR_ELEM;
}

void OperatorClassElement::setOperatorFamily(OperatorFamily *op_family)
{
	if(this->element_type==OPERATOR_ELEM)
	{
		if(op_family && op_family->getIndexingType()!=IndexingType::btree)
			throw Exception(ERR_ASG_INV_OPFAM_OPCLSELEM,__PRETTY_FUNCTION__,__FILE__,__LINE__);

		this->op_family=op_family;
	}
}

void OperatorClassElement::setStorage(PgSQLType storage)
{
	//Clear the attributes not related to the STORAGE element type
	this->function=nullptr;
	this->_operator=nullptr;
	this->op_family=nullptr;
	this->strategy_number=0;

	//Configure the attributes that belongs to the element type
	this->storage=storage;
	this->element_type=STORAGE_ELEM;
}

unsigned OperatorClassElement::getElementType(void)
{
	return(element_type);
}

Function *OperatorClassElement::getFunction(void)
{
	return(function);
}

Operator *OperatorClassElement::getOperator(void)
{
	return(_operator);
}

OperatorFamily *OperatorClassElement::getOperatorFamily(void)
{
	return(op_family);
}

PgSQLType OperatorClassElement::getStorage(void)
{
	return(storage);
}

unsigned OperatorClassElement::getStrategyNumber(void)
{
	return(strategy_number);
}

QString OperatorClassElement::getCodeDefinition(unsigned def_type)
{
	SchemaParser schparser;
	attribs_map attributes;

	attributes[ParsersAttributes::TYPE]=QString();
	attributes[ParsersAttributes::STRATEGY_NUM]=QString();
	attributes[ParsersAttributes::SIGNATURE]=QString();
	attributes[ParsersAttributes::FUNCTION]=QString();
	attributes[ParsersAttributes::OPERATOR]=QString();
	attributes[ParsersAttributes::STORAGE]=QString();
	attributes[ParsersAttributes::OP_FAMILY]=QString();
	attributes[ParsersAttributes::DEFINITION]=QString();

	if(element_type==FUNCTION_ELEM && function && strategy_number > 0)
	{
		//FUNCTION support_number [ ( op_type [ , op_type ] ) ] funcname ( argument_type [, ...] )
		attributes[ParsersAttributes::FUNCTION]=ParsersAttributes::_TRUE_;
		attributes[ParsersAttributes::STRATEGY_NUM]=QString("%1").arg(strategy_number);

		if(def_type==SchemaParser::SQL_DEFINITION)
			attributes[ParsersAttributes::SIGNATURE]=function->getSignature();
		else
			attributes[ParsersAttributes::DEFINITION]=function->getCodeDefinition(def_type,true);
	}
	else if(element_type==OPERATOR_ELEM && _operator && strategy_number > 0)
	{
		//OPERATOR strategy_number operator_name [ ( op_type, op_type ) ] [ FOR SEARCH | FOR ORDER BY sort_family_name ]
		attributes[ParsersAttributes::OPERATOR]=ParsersAttributes::_TRUE_;
		attributes[ParsersAttributes::STRATEGY_NUM]=QString("%1").arg(strategy_number);

		if(def_type==SchemaParser::SQL_DEFINITION)
			attributes[ParsersAttributes::SIGNATURE]=_operator->getSignature();
		else
			attributes[ParsersAttributes::DEFINITION]=_operator->getCodeDefinition(def_type,true);

		if(op_family)
		{
			if(def_type==SchemaParser::SQL_DEFINITION)
				attributes[ParsersAttributes::OP_FAMILY]=op_family->getName(true);
			else
				attributes[ParsersAttributes::DEFINITION]+=op_family->getCodeDefinition(def_type,true);
		}
	}
	else if(element_type==STORAGE_ELEM && storage!=PgSQLType::null)
	{
		//STORAGE storage_type
		attributes[ParsersAttributes::STORAGE]=ParsersAttributes::_TRUE_;

		if(def_type==SchemaParser::SQL_DEFINITION)
			attributes[ParsersAttributes::TYPE]=(*storage);
		else
			attributes[ParsersAttributes::DEFINITION]=storage.getCodeDefinition(def_type);
	}

	return(schparser.getCodeDefinition(ParsersAttributes::ELEMENT,attributes, def_type));
}

bool OperatorClassElement::operator == (OperatorClassElement &elem)
{
	return(this->element_type == elem.element_type &&
		   this->storage == elem.storage &&
		   this->function == elem.function &&
		   this->_operator == elem._operator &&
		   this->strategy_number == elem.strategy_number &&
		   this->op_family == elem.op_family);
}

