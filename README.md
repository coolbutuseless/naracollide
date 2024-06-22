
<!-- README.md is generated from README.Rmd. Please edit that file -->

# naracollide

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/naracollide/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/naracollide/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{naracollide}` is for detecting collisions between native raster
objects

### What’s in the box

- `col_detect_broad()` collision detection between the bounding boxes of
  the native rasters. Fast.
- `col_detect_narrow()` collision detection between overlapping
  non-transparent pixels
- `col_setup()` Setup the collision data structure

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/naracollide) with:

``` r
# install.packages('remotes')
remotes::install_github('coolbutuseless/nara')
remotes::install_github('coolbutuseless/naracollide')
```

## Collision detection example

Does the R logo intersect with any of the circle targets?

``` r
library(nara)
library(naracollide)

# Setup the screen to draw on
screen <- nr_new(200, 200, 'grey90')

# The R logo as a native raster
logo  <- png::readPNG(system.file("img", "Rlogo.png", package="png"), native = TRUE)
xlogo <- 100
ylogo <-  90

# Set up the 4 circular targets
targets <- lapply(1:4, function(i) {
  target <- nr_new(21, 21, 'transparent')
  nr_circle(target, 10, 10, 10, c('firebrick', 'darkblue', 'darkgreen', 'black')[i])
})

xtargets <- c(50,  50, 150, 150)
ytargets <- c(50, 150,  50, 150)

# Draw the targets
for (i in seq_along(targets)) {
  nr_blit(screen, xtargets[i], ytargets[i], targets[[i]], hjust = 0.5, vjust = 0.5)
}

# Draw the logo
nr_blit(screen, xlogo, ylogo, logo, hjust = 0.5, vjust = 0.5)

# Show the screen
plot(screen, T)
```

<img src="man/figures/README-example-1.png" width="100%" />

``` r
# Setup collision detection
coldf <- col_setup(targets, xtargets, ytargets, hjust = 0.5, vjust = 0.5)

# Check for bounding box intersection
# Two `TRUE` values so there are 2 intersections
# out of the possible 4 objects to intersect with.
# Intersection is with the 1st and 3rd targets
col_detect_broad(logo, xlogo, ylogo, coldf, hjust = 0.5, vjust = 0.5)
```

    #> [1]  TRUE FALSE  TRUE FALSE

``` r
# Check for overlapping pixels  
# All values are `FALSE` as there are no overlapping pixels
col_detect_narrow(logo, xlogo, ylogo, coldf, hjust = 0.5, vjust = 0.5)
```

    #> [1] FALSE FALSE FALSE FALSE

### View bounding boxes

When viewed with explicit bounding boxes for each object it is easier to
see:

- The logo’s bounding box intersects the red and green circle - hence
  there are two `TRUE` values returned by `col_detect_broad()`
- The logo itself does not intersect anything - meaning that all values
  returned by `col_detect_narrow()` are `FALSE`

``` r
nr_rect(screen, xlogo, ylogo, ncol(logo), nrow(logo), fill = NA, color = 'black', hjust = 0.5, vjust = 0.5)
nr_rect(screen, xtargets, ytargets, 21, 21, fill = NA, color = 'grey50', hjust = 0.5, vjust = 0.5)

plot(screen, T)
```

<img src="man/figures/README-unnamed-chunk-2-1.png" width="100%" />

## Benchmark

Search for collisions of a `primary` native raster against a backdrop of
100 `secondary` native rasters.

For moving `secondary` objects - where the `col_setup()` structure needs
to be created each time, checking for collisions runs about 50k
times/second (Apple M2).

When the `secondary` objects are known to be stationary (and the
`col_setup()` structure only needs to be created once), checking for
collisions runs about 300k times/second (Apple M2).

``` r
set.seed(1)
library(nara)
library(naracollide)

# Setup primary sprite
primary <- nr_new(100, 100)

# Setup 100 secondary sprites
N <- 100
secondary <- lapply(seq_len(N), function(x) nr_new(20, 20))
x <- sample(200, N, T)
y <- sample(200, N, T)

# Setup the collision data structure and look for collisions
coldf <- col_setup(nr_list = secondary, x = x, y = y)
col_detect_narrow(primary, 0, 0, coldf)
```

    #>   [1]  TRUE FALSE FALSE FALSE FALSE  TRUE FALSE FALSE  TRUE  TRUE FALSE FALSE
    #>  [13]  TRUE  TRUE  TRUE FALSE  TRUE FALSE FALSE FALSE FALSE FALSE FALSE FALSE
    #>  [25] FALSE  TRUE FALSE FALSE FALSE  TRUE  TRUE FALSE FALSE FALSE FALSE  TRUE
    #>  [37] FALSE FALSE FALSE  TRUE FALSE FALSE FALSE  TRUE FALSE FALSE FALSE  TRUE
    #>  [49] FALSE FALSE FALSE  TRUE FALSE  TRUE FALSE FALSE FALSE  TRUE FALSE FALSE
    #>  [61] FALSE  TRUE FALSE FALSE FALSE FALSE FALSE FALSE FALSE  TRUE  TRUE FALSE
    #>  [73]  TRUE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE
    #>  [85] FALSE FALSE FALSE  TRUE FALSE FALSE  TRUE FALSE FALSE  TRUE FALSE FALSE
    #>  [97] FALSE  TRUE FALSE FALSE

``` r
# If secondary objects move, then need to run `col_setup()` each frame
bench::mark({
  coldf <- col_setup(nr_list = secondary, x = x, y = y)
  col_detect_narrow(primary, 0, 0, coldf)
}) %>% select(1:5) %>% knitr::kable()
```

| expression | min | median | itr/sec | mem_alloc |
|:---|---:|---:|---:|---:|
| { coldf \<- col_setup(nr_list = secondary, x = x, y = y) col_detect_narrow(pri… | 11.4µs | 15.2µs | 62238.38 | 61.6KB |

``` r
# If the 'secondary' objects are stationary, the `col_setup()` does not
# need to be run each time.
bench::mark({
  col_detect_narrow(primary, 0, 0, coldf)
}) %>% select(1:5) %>% knitr::kable()
```

| expression                                  |   min | median |  itr/sec | mem_alloc |
|:--------------------------------------------|------:|-------:|---------:|----------:|
| { col_detect_narrow(primary, 0, 0, coldf) } | 1.6µs | 2.05µs | 408200.7 |    10.2KB |

## Animated Example

<figure>
<img src="man/figures/logo-bounce.gif" alt="logo bounce" />
<figcaption aria-hidden="true">logo bounce</figcaption>
</figure>

``` r
library(nara)
library(naracollide)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Load the logo image as a native raster
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
logo <- png::readPNG(system.file("img", "Rlogo.png", package="png"), native = TRUE)
lw <- ncol(logo)
lh <- nrow(logo)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Screen size
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
w <- 800
h <- 600

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Open a window to draw on. 'dbcairo' = double-buffered window
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
x11(type = 'dbcairo')
dev.control(displaylist = 'inhibit')

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Initialise position and velocity of logos
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Nlogos <- 10

x <- sample(0:(w - lw), Nlogos, replace = TRUE)
y <- sample(0:(h - lh), Nlogos, replace = TRUE)

vx <- runif(Nlogos, -5, 5)
vy <- runif(Nlogos, -5, 5)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Initialise screen
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
screen <- nr_new(w, h, 'black')
Nframes <- 100

# If you want to save the frames and output an animation at the end
save_anim <- FALSE
if (save_anim) {
  frames <- vector('list', Nframes)
}

logos <- vector('list', Nlogos)
for (i in seq_len(Nlogos)) {
  logos[[i]] <- nr_duplicate(logo)
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Run the animation
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
for (frame in seq(Nframes)) {
  
  # Clear the screen
  nr_fill(screen, 'black')
  
  # Draw each logo and draw a red border around it if it intersects another logo
  coldf <- col_setup(logos, x, y)
  for (idx in seq_len(Nlogos)) {
    overlap <- col_detect_narrow(logos[[idx]], x[[idx]], y[[idx]], coldf)
    overlap <- setdiff(which(overlap), idx) # exclude self
    # overlap <- 1
    nr_blit(screen, x[[idx]], y[[idx]], logo)
    if (length(overlap)) {
      nr_rect(screen, x[[idx]], y[[idx]], ncol(logo), nrow(logo), fill = NA, color = 'red')
    }
  }
  
  # Update position
  x <- x + vx
  y <- y + vy
  
  # If logo has hit edge of screen, reverse velocity
  vy <- ifelse(y > h - lh | y < 0, -vy, vy)
  vx <- ifelse(x > w - lw | x < 0, -vx, vx)
  
  # Draw to screen
  dev.hold()
  plot(screen)
  dev.flush()
  
  # Keep frame if saving animation at end
  if (save_anim) frames[[frame]] <- nr_duplicate(screen)
  
  Sys.sleep(0.03)
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Create an animation to post to Mastodon :)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (save_anim) {
  nrs_to_gif(frames, gif_name = "man/figures/logo-bounce.gif")
}
```
