
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP col_create_mask_(SEXP nr_) {
  
  int N = length(nr_);
  
  SEXP this_mask_ = PROTECT(allocVector(RAWSXP,N));
  
  uint32_t *nr = (uint32_t *)INTEGER(nr_);
  uint8_t *ptr = RAW(this_mask_);
  for (int i = 0; i < N; i++) {
    ptr[i] = ((nr[i] >> 24) & 0xFF) != 0;
  }
  
  UNPROTECT(1);
  return this_mask_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup collision structure
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP col_setup_(SEXP nr_list_, SEXP x_, SEXP y_, SEXP hjust_, SEXP vjust_) {
  
  int nprotect = 0;
  int N = length(nr_list_);
  if (N != length(x_) || N != length(y_)) {
    error("'nr_list', 'x' and 'y', must all be the same length");
  }
  
  double hjust = asReal(hjust_);
  double vjust = asReal(vjust_);
  
  SEXP x0_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP y0_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP x1_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP y1_   = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP w_    = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP h_    = PROTECT(allocVector(INTSXP, N)); nprotect++;
  SEXP mask_ = PROTECT(allocVector(VECSXP, N)); nprotect++;
  
  int *x  = INTEGER(x_);
  int *y  = INTEGER(y_);
  int *x0 = INTEGER(x0_);
  int *y0 = INTEGER(y0_);
  int *x1 = INTEGER(x1_);
  int *y1 = INTEGER(y1_);
  int *w  = INTEGER(w_);
  int *h  = INTEGER(h_);
  
  for (int i = 0; i < N; i++) {
    SEXP nr_ = VECTOR_ELT(nr_list_, i);
    
    w[i]  = Rf_ncols(nr_);
    h[i]  = Rf_nrows(nr_);
    x0[i] = (int)round(x[i] - hjust * w[i]);
    y0[i] = (int)round(y[i] - vjust * h[i]);
    x1[i] = x0[i] + w[i] - 1;
    y1[i] = y0[i] + h[i] - 1;
    
    
    SET_VECTOR_ELT(mask_, i, col_create_mask_(nr_));
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup an empty data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int ncols = 7;
  SEXP df_  = PROTECT(allocVector(VECSXP, ncols)); nprotect++;
  SEXP nms_ = PROTECT(allocVector(STRSXP, ncols)); nprotect++;
  SET_STRING_ELT(nms_, 0, mkChar("x0"));
  SET_STRING_ELT(nms_, 1, mkChar("y0"));
  SET_STRING_ELT(nms_, 2, mkChar("x1"));
  SET_STRING_ELT(nms_, 3, mkChar("y1"));
  SET_STRING_ELT(nms_, 4, mkChar("w"));
  SET_STRING_ELT(nms_, 5, mkChar("h"));
  SET_STRING_ELT(nms_, 6, mkChar("nask"));
  setAttrib(df_, R_NamesSymbol, nms_);
  setAttrib(df_, R_ClassSymbol, mkString("data.frame"));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the row.names on the list.
  // Use the shortcut as used in .set_row_names() in R
  // i.e. set rownames to c(NA_integer, -len) and it will
  // take care of the rest. This is equivalent to rownames(x) <- NULL
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP rownames = PROTECT(allocVector(INTSXP, 2));  nprotect++;
  SET_INTEGER_ELT(rownames, 0, NA_INTEGER);
  SET_INTEGER_ELT(rownames, 1, -N);
  setAttrib(df_, R_RowNamesSymbol, rownames);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set all the columns
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SET_VECTOR_ELT(df_, 0, x0_);
  SET_VECTOR_ELT(df_, 1, y0_);
  SET_VECTOR_ELT(df_, 2, x1_);
  SET_VECTOR_ELT(df_, 3, y1_);
  SET_VECTOR_ELT(df_, 4, w_);
  SET_VECTOR_ELT(df_, 5, h_);
  SET_VECTOR_ELT(df_, 6, mask_);
  
  
  UNPROTECT(nprotect);
  return df_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP col_detect_broad_(SEXP nr_, SEXP x_, SEXP y_, SEXP cldf_, SEXP hjust_, SEXP vjust_) {
  
  int w = Rf_ncols(nr_);
  int h = Rf_nrows(nr_);
  
  double hjust = asReal(hjust_);
  double vjust = asReal(vjust_);
  
  int x = asInteger(x_) - round(hjust * w);
  int y = asInteger(y_) - round(vjust * h);
  
  SEXP x0_   = VECTOR_ELT(cldf_, 0);
  SEXP y0_   = VECTOR_ELT(cldf_, 1);
  SEXP x1_   = VECTOR_ELT(cldf_, 2);
  SEXP y1_   = VECTOR_ELT(cldf_, 3);
  // SEXP w_    = VECTOR_ELT(cldf_, 4);
  // SEXP h_    = VECTOR_ELT(cldf_, 5);
  // SEXP mask_ = VECTOR_ELT(cldf_, 6);
  
  int *x0 = INTEGER(x0_);
  int *y0 = INTEGER(y0_);
  int *x1 = INTEGER(x1_);
  int *y1 = INTEGER(y1_);
  
  // Row in data.frame
  int N = length(VECTOR_ELT(cldf_, 0));
  
  SEXP overlap_ = PROTECT(allocVector(LGLSXP, N));
  int *overlap = LOGICAL(overlap_);

  for (int i = 0; i < N; i++) {
    overlap[i] = !(
        x         > x1[i] |
        x + w - 1 < x0[i] |
        y         > y1[i] |
        y + h - 1 < y0[i]
    );
  }
  
  UNPROTECT(1);
  return overlap_;
}


#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)

    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP col_detect_narrow_(SEXP nr_, SEXP x_, SEXP y_, SEXP cldf_, SEXP hjust_, SEXP vjust_) {
  
  int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get broad overlap for overlapping rectangles
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP overlap_ = PROTECT(col_detect_broad_(nr_, x_, y_, cldf_, hjust_, vjust_)); nprotect++;
  int *overlap = LOGICAL(overlap_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack info about the primary sprite
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int w = Rf_ncols(nr_);
  int h = Rf_nrows(nr_);
  
  double hjust = asReal(hjust_);
  double vjust = asReal(vjust_);
  
  int x = asInteger(x_) - round(hjust * w);
  int y = asInteger(y_) - round(vjust * h);
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Sanity check
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int N = length(VECTOR_ELT(cldf_, 0));
  if (N != length(overlap_)) {
    error("col_detect_narrow_(): length mismatch");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack collision data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP x0_   = VECTOR_ELT(cldf_, 0);
  SEXP y0_   = VECTOR_ELT(cldf_, 1);
  SEXP x1_   = VECTOR_ELT(cldf_, 2);
  SEXP y1_   = VECTOR_ELT(cldf_, 3);
  SEXP sw_   = VECTOR_ELT(cldf_, 4);
  // SEXP h_    = VECTOR_ELT(cldf_, 5);
  SEXP mask_ = VECTOR_ELT(cldf_, 6);
  
  int *x0 = INTEGER(x0_);
  int *y0 = INTEGER(y0_);
  int *x1 = INTEGER(x1_);
  int *y1 = INTEGER(y1_);
  int *sw = INTEGER(sw_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // iterate over collisions which matter
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP pmask_ = PROTECT(col_create_mask_(nr_)); nprotect++;
  uint8_t *pmask = RAW(pmask_);
  
  for (int i = 0; i < N; i++) {
    if (!overlap[i]) continue;
    
    // Rprintf("Primary overlaps with %i\n", i);
    
    uint8_t *smask = RAW(VECTOR_ELT(mask_, i));  
    
    int xmin = max(x        , x0[i]);
    int xmax = min(x + w - 1, x1[i]);
    
    int ymin = max(y        , y0[i]);
    int ymax = min(y + h - 1, y1[i]);
    
    // Rprintf("screen: (%i, %i) (%i, %i)\n", xmin        , ymin        , xmax - xmin, ymax - ymin);
    // Rprintf("prim  : (%i, %i) (%i, %i)\n", xmin - x    , ymin - y    , xmax - xmin, ymax - ymin);
    // Rprintf("second: (%i, %i) (%i, %i)\n", xmin - x0[i], ymin - y0[i], xmax - xmin, ymax - ymin);
    
    int collision = 0;
    for (int xoff = 0; xoff < (xmax - xmin); xoff++) {
      int px = xmin - x     + xoff;
      int sx = xmin - x0[i] + xoff;
      for (int yoff = 0; yoff < (ymax - ymin); yoff++) {
        int py = ymin - y     + yoff;
        int sy = ymin - y0[i] + yoff;
        int primary   = pmask[py *  w    + px];
        int secondary = smask[sy * sw[i] + sx];
        if (primary && secondary) {
          collision = 1;
          break;
        }
      }
    }
    
    overlap[i] = collision;
  }
  
  
  
  UNPROTECT(nprotect);
  return overlap_;
}


