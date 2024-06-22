
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create a mask of the non-transparent pixels
#' @param nr nativeraster
#' @return raw vector with each byte being either 0 (for a transparent pixel) or
#'         1 for a non-transparent pixel
#' @examples
#' nr <- nara::nr_new(10, 10, 'white')
#' col_create_mask(nr)
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
col_create_mask <- function(nr) {
  .Call(col_create_mask_, nr);
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Setup the collision structure
#' 
#' @param nr_list list of native rasters
#' @param x,y coordinates for each native raster
#' @param hjust,vjust specify horizontal and vertical justification of the 
#'        raster.  e.g. \code{hjust = vjust = 0} the blitting
#'        starts at the top-left of the image. Use \code{hjust = vjust = 0.5}
#'        to treat the centre of the \code{src_} as the blitting origin.
#'        Default (0, 0)
#' 
#' @return collision data.frame with stored information about all the given 
#'         native rasters.  Each row represents the corresponding object in the
#'         \code{nr_list}
#' @examples
#' nr1 <- nara::nr_new(5, 2, 'white')
#' nr2 <- nara::nr_new(4, 4, 'grey')
#' nr_list <- list(nr1, nr2)
#' coldf <- col_setup(nr_list, x = c(0, 10), y = c(100, 10))
#' coldf
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
col_setup <- function(nr_list, x, y, hjust = 0, vjust = 0) {
  .Call(col_setup_, nr_list, as.integer(round(x)), as.integer(round(y)), hjust, vjust)
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Detect bounding box collision
#'
#' @inheritParams col_setup
#' @param nr primary native raster which will be tested against all the 
#'        native rasters represented in \code{coldf}
#' @param x,y Scalar coordinates of \code{nr}
#' @param coldf collision data.frame returned by \code{col_setup()}
#' @return logical vector with an entry for each row in \code{coldf}; TRUE
#'         when bounding box for \code{nr} overlaps any of the bounding boxes for
#'         the native rasters in \code{coldf}, otherwise FALSE.
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
col_detect_broad <- function(nr, x, y, coldf, hjust = 0, vjust = 0) {
  .Call(col_detect_broad_, nr, x, y, coldf, hjust, vjust);
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Detect overlapping non-transparent pixels
#' 
#' @inheritParams col_detect_broad
#' 
#' @return logical vector with an entry for each row in \code{coldf}; TRUE
#'         when any non-transparent pixel in \code{nr} overlaps any 
#'         non-transparent pixel in 
#'         the native rasters in \code{coldf}, otherwise FALSE.
#' @examples
#' nr1 <- nara::nr_new(5, 2, 'white')
#' nr2 <- nara::nr_new(4, 4, 'grey')
#' nr_list <- list(nr1, nr2)
#' coldf <- col_setup(nr_list, x = c(0, 10), y = c(100, 10))
#' coldf 
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
col_detect_narrow <- function(nr, x, y, coldf, hjust = 0, vjust = 0) {
  .Call(col_detect_narrow_, nr, x, y, coldf, hjust, vjust)
}




if (FALSE) {
  
  library(nara)
  
  w <- 600
  h <- 400
  screen <- nr_new(w, h, 'grey80')
                   
  Nbaddies <- 10
  
  set.seed(1)
  baddies <- lapply(
    seq(Nbaddies), 
    function(x) {
      nr_new(150, 70, sample(colours(), 1))
    }
  )
  
  x <- sample(w, Nbaddies)
  y <- sample(h, Nbaddies)
  
  bench::mark(
    coldf <- col_setup(baddies, x, y)
  )
  
  bench::mark(
    cldf <- cl_setup(baddies, x, y)
  )
  
  for (i in seq(Nbaddies)) {
    nr_blit(screen, x[i], y[i], baddies[[i]])
  }
  
  nr_rect(screen, cldf$x0, cldf$y0, cldf$w, cldf$h, fill = NA, color = 'black')
  
  hero <- png::readPNG(system.file("img", "Rlogo.png", package="png"), native = TRUE)
  
  xhero <- 170L
  yhero <- 100L
  bench::mark(
    cl_detect_broad(hero, xhero, yhero, cldf), 
    col_detect_broad(hero, xhero, yhero, coldf) 
  )
  bench::mark(
    cl_detect_narrow(hero, xhero, yhero, cldf),
    col_detect_narrow(hero, xhero, yhero, coldf) 
  )
  
  nr_blit(screen, xhero, yhero, hero)
  
  
  # nr_rect(screen, xmin, ymin, xmax-xmin+1, ymax-ymin+1, fill = NA, color = 'red')
  
  plot(screen)
    
}


if (FALSE) {
  
  hero <- png::readPNG(system.file("img", "Rlogo.png", package="png"), native = TRUE)
  
  thero <- (hero |> bitwShiftR(24)) == 0
  dim(thero) <- dim(hero)
  class(thero) <- 'nativeRaster'
  plot(thero, T)  
}





