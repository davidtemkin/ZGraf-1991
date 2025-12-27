/*
 * ZGraf - Inheritance.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Inheritance

	// ----------- utility macros
	
		#ifndef nil
			#define nil ((void *) 0L)
		#endif
	
		#define concat2(a, b)					a##b
		#define concat(a, b)					concat2(a, b)
	
	
 	// ----------- externally defined heap functions

 		#define allocMem(bytes)			ZNewPtr(bytes)
 		#define freeMem(ptr)			DisposePtr((Ptr) ptr )


	// ----------- macros for declarations and definitions of classes and instances

		#define declareRootClass(_class)												\
					typedef struct concat(_class, Class) {								\
						void *superClass;												\
						concat(_class, _classDeclarations)								\
						} concat(_class, Class);										\
					typedef struct _class {												\
						concat(_class, Class) *theClass;								\
						concat(_class, _instanceDeclarations)							\
						} _class;														\
					extern concat(_class, Class) *concat(concat(g, _class), Class);		\
					extern void concat(_I, concat(_class, Class))						\
						(concat(concat(_class, Class), *))

		#define declareClass(_class, _inheritedClass)									\
					typedef struct concat(_class, Class) {								\
						concat(_inheritedClass, Class) *superClass;						\
						concat(_class, _classDeclarations)								\
						} concat(_class, Class);										\
					typedef struct _class {												\
						concat(_class, Class) *theClass;								\
						concat(_class, _instanceDeclarations)							\
						} _class;														\
					extern concat(_class,Class) * concat(g, concat(_class, Class));		\
					extern void concat(_I, concat(_class, Class))						\
						(concat(concat(_class, Class), *))

		#define declareMethod(metH)														\
					long (*metH)(...)

		#define defineRootClassObject 													\
					void concat(_I, concat(CLASS, Class))(concat(CLASS, Class) *co)		\
					{ methodDefinitions }												\
					concat(CLASS, Class) *concat(g, concat(CLASS, Class)) = nil

		#define defineMethod(_method)													\
					long concat(_method, concat(_, CLASS))

		#define	definesMethod(_method)													\
					co->_method =														\
						(long (*)(...)) &concat(_method, concat(_, CLASS))

		#define defineClassObject 														\
					void concat(_I, concat(CLASS, Class))(concat(CLASS, Class) *co)		\
					{																	\
					concat(_I, concat(SUPERCLASS, Class))((								\
						concat(SUPERCLASS, Class) *) co);								\
					methodDefinitions													\
					}																	\
					concat(CLASS, Class) *concat(g, concat(CLASS, Class)) = nil


	// ----------- macros for class allocation and initialization

		#define prepareClass(_class, _superClass)										\
						(concat(_class, Class) *)										\
						allocMem(sizeof(concat(_class, Class)));						\
					concat(g, concat(_class, Class))->superClass = 						\
						concat(g, concat(_superClass, Class));							\
					concat(_I, concat(_class, Class))(									\
						concat(g, concat(_class, Class)))

		#define prepareRootClass(_class)												\
						(concat(_class, Class) *)										\
						allocMem(sizeof(concat(_class, Class)));						\
					concat(g, concat(_class, Class))->superClass = nil;					\
					concat(_I, concat(_class, Class))(									\
						concat(g, concat(_class, Class)))


	// ----------- for normal runtime usage

		#define newInstance(_class)														\
					(_class *) _NewInstance(sizeof(_class),								\
						concat(g, concat(_class, Class)))

		#define method(_meth)															\
					self)->theClass->_meth(self

		#define message(_inst, _meth)													\
					_inst)->theClass->_meth(_inst

		#define var(_v)																	\
					(self->_v)

		#define objVar(_obj, _v)														\
					(_obj->_v)

		#define classVar(_cv)															\
					(self->theClass->_cv)

		#define objClassVar(_obj, _cv)													\
					(_obj->theClass->_cv)

		#define inherited(_meth)														\
					concat(g, concat(SUPERCLASS, Class))->_meth)(self

		#define isMember(_inst, _class)													\
					_IsMember((void *) _inst,											\
					(void *) concat(g, concat(_class, Class)))


	// ----------- declarations of object utilities defined in Inheritance.c

		extern long _IsMember(void *instance, void *classObj);
		extern void *_NewInstance(long instSize, void *classObj);

	#define _H_Inheritance

#endif
