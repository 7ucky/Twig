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

#ifndef NURIA_TEMPLATEPROGRAM
#define NURIA_TEMPLATEPROGRAM

#include <nuria/callback.hpp>
#include "templateerror.hpp"
#include "twig_global.hpp"
#include <QSharedData>
#include <QStringList>

namespace Nuria {

class TemplateProgramPrivate;
class TemplateEngine;

/**
 * \brief Represents a compiled Twig code program.
 * 
 * A TemplateProgram stores a Twig program independent from the TemplateEngine
 * used to create it and all other classes.
 * 
 * \note Instances of this class can only be created by TemplateEngine.
 * \note This structure is implicitly shared.
 * 
 * \par Caching
 * 
 * You're free to cache instances of this class wherever you want, though you
 * should be advised that in this case you \b won't be notified when a template
 * loader notifies the application of a changed Twig template.
 * 
 * Because of this, it's advisable to create a TemplateEngine once and keep
 * using it throughout the application. See Nuria::DependencyManager for a
 * easy way of doing this.
 * 
 * \par Copy-Behaviour
 * 
 * This class stores variables implicitly shared, meaning that using setValue()
 * on a instance will change the variable only in this program. Note that this
 * is a comparably cheap operation, as the internal program is stored in its own
 * structure.
 * 
 * \note The internal program is never copied nor changed by any operation.
 * 
 * \par Variables
 * 
 * By default, a program is in strict mode, meaning that all referenced
 * variables and functions must be present upon rendering. If there's a variable
 * without a value set, then render() will fail by returning an empty string and
 * setting the last error accordingly. You can use neededVariables() to get a
 * list of variables needed by the program to execute.
 * 
 * \sa neededVariables canRender render
 * 
 */
class NURIA_TWIG_EXPORT TemplateProgram {
public:
	
	/** Constructor for a invalid instance. */
	TemplateProgram ();
	
	/** Copy constructor. */
	TemplateProgram (const TemplateProgram &other);
	
	/** Assignment operator. */
	TemplateProgram &operator= (const TemplateProgram &other);
	
	/** Destructor. */
	~TemplateProgram ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/**
	 * Returns a list of all templates this program depends on, meaning,
	 * which have been included or extended.
	 */
	QStringList dependencies () const;
	
	/**
	 * Returns a unordered list of variable names needed by this program.
	 */
	QStringList neededVariables () const;
	
	/** Returns the current value of \a variable. */
	QVariant value (const QString &variable) const;
	
	/**
	 * Sets the value of \a variable to \a value. Returns \c true if this
	 * program needs \a variable, else \c false is returned.
	 */
	bool setValue (const QString &variable, const QVariant &value);
	
	/** Returns the locale used by this program. */
	QLocale locale () const;
	
	/** Sets the locale used by this program. */
	void setLocale (const QLocale &locale);
	
	/**
	 * Adds \a function, making it known as \a name.
	 * \note Built-in functions can not be overridden.
	 * 
	 * It's also possible to explicitly define constant functions which
	 * are evaluated (if possible) during compilation. If you want to do
	 * that, please use TemplateEngine::addFunction() instead.
	 * 
	 * \warning While it's possible to override user-defined functions here,
	 * overriding a constant function can have unwanted side-effects.
	 */
	void addFunction (const QString &name, const Callback &function);
	
	/**
	 * Returns \c true, if there's a user-defined function called \a name.
	 */
	bool hasFunction (const QString &name);
	
	/**
	 * Checks if all needed variables are set. If yes, then \c true is
	 * returned. If not, then \c false is returned and lastError() will
	 * return the TemplateError indicating which variable is missing.
	 * 
	 * \sa neededVariables
	 */
	bool canRender () const;
	
	/**
	 * Executes the program and returns the result.
	 * A empty result indicates an error.
	 * 
	 * \sa lastError
	 */
	QString render ();
	
	/** Returns the last error. */
	TemplateError lastError () const;
	
private:
	friend class TemplateEngine;
	
	TemplateProgram (TemplateProgramPrivate *dptr);
	void refNode ();
	void derefNode ();
	bool checkVariable (int index) const;
	
	QSharedDataPointer< TemplateProgramPrivate > d;
	
};

}

Q_DECLARE_METATYPE(Nuria::TemplateProgram)

#endif // NURIA_TEMPLATEPROGRAM
