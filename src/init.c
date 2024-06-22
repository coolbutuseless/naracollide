
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP col_setup_(SEXP nr_list_, SEXP x_, SEXP y_);
extern SEXP col_create_mask_(SEXP nr_);

extern SEXP col_detect_broad_ (SEXP nr_, SEXP x_, SEXP y_, SEXP cldf_);
extern SEXP col_detect_narrow_(SEXP nr_, SEXP x_, SEXP y_, SEXP cldf_);

static const R_CallMethodDef CEntries[] = {
  
  {"col_setup_"        , (DL_FUNC) &col_setup_        , 3},
  {"col_create_mask_"  , (DL_FUNC) &col_create_mask_  , 1},
  {"col_detect_broad_" , (DL_FUNC) &col_detect_broad_ , 4},
  {"col_detect_narrow_", (DL_FUNC) &col_detect_narrow_, 4},
  
  {NULL , NULL, 0}
};


void R_init_naracollide(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}



