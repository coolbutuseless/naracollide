


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create a transparency mask for a sprite
#' 
#' @param sprite nativeraster
#' @return transparency mask
#' 
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cl_create_mask <- function(sprite) {
  this_mask <- (sprite |> bitwShiftR(24)) > 0
  dim(this_mask) <- dim(sprite)
  class(this_mask) <- 'nativeRaster'
  this_mask
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Setup
#' @param baddies baddies
#' @param x,y coordinates
#' 
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cl_setup <- function(baddies, x, y) {
  
  w <- vapply(baddies, ncol, integer(1))
  h <- vapply(baddies, nrow, integer(1))
  
  mask <- lapply(baddies, cl_create_mask)
  
  structure(
    list(
      x0 = x,
      y0 = y,
      x1 = x + w - 1L,
      y1 = y + h - 1L,
      w  = w,
      h  = h,
      mask = mask
    ),
    
    class     = 'data.frame',
    row.names = .set_row_names(length(baddies))
  )
  
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Detect collision - broad
#' 
#' @param hero hero
#' @param x,y coords
#' @param cldf collision data.frame
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cl_detect_broad <- function(hero, x, y, cldf) {
  !(
    (x                  > cldf$x1) |
      (x + ncol(hero) - 1 < cldf$x0) |
      (y                  > cldf$y1) |
      (y + nrow(hero) - 1 < cldf$y0)
  )
}



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' @rdname cl_detect_broad
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cl_detect_narrow <- function(hero, xhero, yhero, cldf) {
  overlap <- cl_detect_broad(hero, xhero, yhero, cldf)
  
  indices <- which(overlap)
  idx <- indices[1]  
  
  for (idx in indices) {
    
    xmin <- max(xhero                 , cldf$x0[idx])
    xmax <- min(xhero + ncol(hero) - 1, cldf$x1[idx])
    
    ymin <- max(yhero                 , cldf$y0[idx])
    ymax <- min(yhero + nrow(hero) - 1, cldf$y1[idx])
    
    hero_mask <- cl_create_mask(hero) 
    
    if (requireNamespace('nara', quietly = TRUE)) {
      sub_hero <- nara::nr_crop(hero_mask, xmin - xhero, ymin - yhero, xmax - xmin, ymax - ymin)
      
      baddy <- cldf[idx, ]
      
      sub_baddy <- nara::nr_crop(baddy$mask[[1]], xmin - baddy$x0, ymin - baddy$y0, 
                           w = xmax - xmin, h = ymax - ymin)
      
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # We now have enough information for a detailed overlap
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      overlap[idx] <- any(sub_hero & sub_baddy)
    } else {
      stop("Need {nara} installed for this")
    }
  }
  
  overlap
}