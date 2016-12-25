//
//  rayjsonpick.cpp
//
//  Created by leico on 2016/12/23.
//
//

#include <iostream>
#include <string>
#include <sstream>
#include <picojson.h>

#include "ext.h"
#include "ext_obex.h"

#define PICOJSON_USE_INT64

typedef struct _rayJsonPick{
  t_object  object;
  void     **out;

  long  holetotal;

  long   inletnum;
  void** proxy;

  t_symbol **target;

} rayJsonPick;

void* newObject (t_symbol *s, long argc, t_atom *argv);
void  freeObject(rayJsonPick *x);
void  assistNavi(rayJsonPick *x, void *b, long m, long a, char *s);
void  getSymbol(rayJsonPick *x, t_symbol *s, long argc, t_atom *argv);

t_symbol* toSymbol(rayJsonPick *x, const t_atom* target);


void *class_rayJsonPick;

void ext_main(void *r){

  t_class *c;

  c = class_new("ray.jsonpick", (method)newObject, (method)freeObject, (long)sizeof(rayJsonPick), 0L, A_GIMME, 0);

  class_addmethod(c, (method)assistNavi, "assist", A_CANT, 0);
  class_addmethod(c, (method)getSymbol,  "anything", A_GIMME, 0);

  class_register(CLASS_BOX, c);
  class_rayJsonPick = c;
}


void* newObject (t_symbol *s, long argc, t_atom *argv){

  rayJsonPick *x = NULL;

  if( (x = (rayJsonPick *)object_alloc((t_class *)class_rayJsonPick)) ){

    object_post((t_object *)x, "%s object instance created", s -> s_name);


    x -> target = (t_symbol **)malloc(sizeof(t_symbol *) * argc);
    x -> out    = (void **)    malloc(sizeof(void *)     * argc);
    x -> proxy  = (void **)    malloc(sizeof(void *)     * argc);
    for(int i = argc - 1 ; i >= 0 ; -- i){
      x -> target[i] = toSymbol(x, &argv[i]);
      x -> out   [i] = outlet_new(x, NULL);
      x -> proxy [i] = proxy_new( (t_object *)x, i + 1, &(x -> inletnum) );
    }

    x -> holetotal = argc;
  }

  return x;

}
void freeObject(rayJsonPick *x){

  free(x -> target);
  free(x -> out);
  free(x -> proxy);


}


void  getSymbol(rayJsonPick *x, t_symbol *s, long argc, t_atom *argv){

  using jsonobject = picojson :: object;

  int num = proxy_getinlet((t_object *)x);

  if(num == 0){
    picojson :: value json;

    std :: string err = picojson :: parse(json, s -> s_name);
    if( ! err.empty()){
      object_error((t_object *)x, "picojson parse error ! not JSON string");
      return;
    }

    if( ! json.is<picojson :: object>() ){
      object_warn((t_object *)x, "data is not object, you can decode any other max object");
      return;
    }

    const jsonobject& obj = json.get<picojson::object>();

    for(int i = 0 ; i < x -> holetotal ; ++ i){
      jsonobject :: const_iterator it = obj.find(x -> target[i] -> s_name);
      if( it != obj.end() ){
        outlet_anything(x -> out[i], gensym( it -> second.serialize().c_str() ), 0, (t_atom *)NIL);
      }
    }
  }
  else
    x -> target[num - 1] = s;
}

void assistNavi(rayJsonPick *x, void *b, long m, long a, char *s){

  if (m == ASSIST_INLET) { // inlet
    sprintf(s, "I am inlet %ld", a - 1);
  }
  else {	// outlet
    sprintf(s, "I am outlet %ld", a);
  }

}

t_symbol* toSymbol(rayJsonPick *x, const t_atom* target){

  std :: stringstream ss;
  switch(atom_gettype(target)){
    case A_LONG :
      ss << atom_getlong(target);
      return gensym(ss.str().c_str());
    break;

    case A_FLOAT:
      ss << atom_getfloat(target);
      return gensym(ss.str().c_str());
    break;

    case A_SYM:
      return atom_getsym(target);
    break;

    default:
      object_warn((t_object *)x, "not supported value");
      return gensym("");
   break; 
  }
}
