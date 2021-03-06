#include <common/utils.h>
#include <php7/utils.h>

#include "html5_dom_exception.h"
#include "html5_dom_collection.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"

/*
	Class HTML5\DOM\Collection
*/

static zend_object_handlers php_html5_dom_collection_handlers;
static HashTable php_html5_dom_collection_prop_handlers;

ZEND_BEGIN_ARG_INFO_EX(arginfo_collection_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_collection_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_collection_offset_value, 0, 0, 2)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static zend_function_entry php_html5_dom_collection_methods[] = {
	PHP_ME(Collection, __construct,		arginfo_collection_void,			ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
	PHP_ME(Collection, count,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, offsetExists,	arginfo_collection_offset,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, offsetGet,		arginfo_collection_offset,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, offsetSet,		arginfo_collection_offset_value,	ZEND_ACC_PUBLIC)
	PHP_ME(Collection, offsetUnset,		arginfo_collection_offset,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, next,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, key,				arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, valid,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, rewind,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, current,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, array,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, first,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	PHP_ME(Collection, last,			arginfo_collection_void,			ZEND_ACC_PUBLIC)
	
	// aliases
	PHP_MALIAS(Collection,	item,		offsetGet,			arginfo_collection_offset,		ZEND_ACC_PUBLIC)
	PHP_MALIAS(Collection,	length,		count,				arginfo_collection_void,		ZEND_ACC_PUBLIC)
	PHP_MALIAS(Collection,	toArray,	array,				arginfo_collection_void,		ZEND_ACC_PUBLIC)
	
	PHP_FE_END
};

void html5_dom_collection_class_init() {
	zend_class_entry ce;
	memcpy(&php_html5_dom_collection_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_html5_dom_collection_handlers.offset				= XtOffsetOf(html5_dom_object_wrap, std);
	php_html5_dom_collection_handlers.free_obj				= html5_dom_collection_free_obj;
	php_html5_dom_collection_handlers.clone_obj				= NULL;
	
	php_html5_dom_collection_handlers.read_property			= html5_dom_read_property;
	php_html5_dom_collection_handlers.write_property		= html5_dom_write_property;
	php_html5_dom_collection_handlers.get_property_ptr_ptr	= html5_dom_get_property_ptr_ptr;
	php_html5_dom_collection_handlers.has_property			= html5_dom_has_property;
	php_html5_dom_collection_handlers.get_debug_info		= html5_dom_get_debug_info;
	
	INIT_CLASS_ENTRY(ce, "HTML5\\DOM\\Collection", php_html5_dom_collection_methods);
	ce.create_object = html5_dom_collection_create_object;
	html5_dom_collection_ce = zend_register_internal_class(&ce);
	
	zend_class_implements(html5_dom_collection_ce, 3, zend_ce_countable, zend_ce_arrayaccess, zend_ce_iterator);
	
	html5_dom_prop_handler_list handlers[] = {
		{"length",			html5_dom_collection__length}, 
		{"items",			html5_dom_collection__items}, 
		{"",				NULL}, 
	};
	html5_dom_prop_handler_init(&php_html5_dom_collection_prop_handlers, handlers);
}

static zend_object *html5_dom_collection_create_object(zend_class_entry *ce TSRMLS_DC) {
	html5_dom_object_wrap *intern = html5_dom_object_wrap_create(ce, &php_html5_dom_collection_handlers);
	
	intern->prop_handler = &php_html5_dom_collection_prop_handlers;
	intern->iter = 0;
	
	DOM_GC_TRACE("DOM::Collection::new (refs=%d)", GC_REFCOUNT(&intern->std));
	
	return &intern->std;
}

static void html5_dom_collection_free_obj(zend_object *object TSRMLS_DC) {
	html5_dom_object_wrap *intern = html5_dom_object_unwrap(object);
	
	DOM_GC_TRACE("DOM::Collection::DESTROY (refs=%d)", GC_REFCOUNT(&intern->std));
	
	if (intern->ptr) {
		myhtml_collection_destroy((myhtml_collection_t *) intern->ptr);
		zval_ptr_dtor(&intern->parent);
	}
	
	html5_dom_object_wrap_free(intern);
}

void html5_dom_collection_class_unload() {
	zend_hash_destroy(&php_html5_dom_collection_prop_handlers);
}

void html5_dom_collection_to_zval(myhtml_tree_t *tree, myhtml_collection_t *collection, zval *retval) {
	// new HTML5\DOM\Collection
	object_init_ex(retval, html5_dom_collection_ce);
	
	html5_dom_object_wrap *intern = html5_dom_object_unwrap(Z_OBJ_P(retval));
	intern->ptr = collection;
	
	// Save reference to parent
	// That would not deleted tree object while at least one collection object live
	html5_dom_tree_t *tree_obj = (html5_dom_tree_t *) tree->context;
	html5_dom_object_wrap *tree_intern = (html5_dom_object_wrap *) tree_obj->sv;
	
	GC_REFCOUNT(&tree_intern->std)++;
	ZVAL_OBJ(&intern->parent, &tree_intern->std);
}

/*
	PHP methods
*/
PHP_METHOD(Collection, __construct) {
	
}

PHP_METHOD(Collection, array) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	array_init(return_value);
	for (size_t i = 0; i < self->length; ++i) {
		zval tmp;
		html5_dom_node_to_zval(self->list[0], &tmp);
		add_index_zval(return_value, i, &tmp);
	}
}

PHP_METHOD(Collection, first) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	if (self->length > 0) {
		html5_dom_node_to_zval(self->list[0], return_value);
	} else {
		RETURN_NULL();
	}
}

PHP_METHOD(Collection, last) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	if (self->length > 0) {
		html5_dom_node_to_zval(self->list[self->length - 1], return_value);
	} else {
		RETURN_NULL();
	}
}

PHP_METHOD(Collection, count) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	RETURN_LONG(self->length);
}

PHP_METHOD(Collection, offsetExists) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	zend_long offset = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) != SUCCESS)
		WRONG_PARAM_COUNT;
	
	RETURN_BOOL(offset >= 0 && offset < self->length);
}

PHP_METHOD(Collection, offsetGet) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	zend_long offset = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) != SUCCESS)
		WRONG_PARAM_COUNT;
	
	if (offset >= 0 && offset < self->length) {
		html5_dom_node_to_zval(self->list[self_object->iter], return_value);
	} else {
		RETURN_NULL();
	}
}

PHP_METHOD(Collection, offsetSet) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	zend_throw_exception_ex(html5_dom_exception_ce, 0, "collection readonly");
}

PHP_METHOD(Collection, offsetUnset) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	zend_throw_exception_ex(html5_dom_exception_ce, 0, "collection readonly");
}

PHP_METHOD(Collection, current) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	
	if (self_object->iter >= 0 && self_object->iter < self->length) {
		html5_dom_node_to_zval(self->list[self_object->iter], return_value);
	} else {
		RETURN_NULL();
	}
}

PHP_METHOD(Collection, next) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	++self_object->iter;
}

PHP_METHOD(Collection, key) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	RETURN_LONG(self_object->iter);
}

PHP_METHOD(Collection, valid) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	RETURN_BOOL(self_object->iter >= 0 && self_object->iter < self->length);
}

PHP_METHOD(Collection, rewind) {
	HTML5_DOM_METHOD_PARAMS(myhtml_collection_t);
	self_object->iter = 0;
}

/*
	Acessors
*/
static int html5_dom_collection__length(html5_dom_object_wrap *obj, zval *val, int write, int debug) {
	myhtml_collection_t *self = (myhtml_collection_t *) obj->ptr;
	
	if (!write) {
		ZVAL_LONG(val, self->length);
		return 1;
	}
	
	return 0;
}

static int html5_dom_collection__items(html5_dom_object_wrap *obj, zval *val, int write, int debug) {
	myhtml_collection_t *self = (myhtml_collection_t *) obj->ptr;
	
	if (!write) {
		array_init(val);
		for (size_t i = 0; i < self->length; ++i) {
			zval tmp;
			html5_dom_node_to_zval(self->list[0], &tmp);
			add_index_zval(val, i, &tmp);
		}
		return 1;
	}
	
	return 0;
}
