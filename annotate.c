/*
 * IPv4 Heatmap
 * (C) 2007 The Measurement Factory, Inc
 * Licensed under the GPL, version 2
 * http://maps.measurement-factory.com/
 */

/*
 * Place annotations (text) on the image.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gd.h>
#include "include/bbox.h"

/*
 * FONT_ALPHA sets the transparency for the annotations. in libgd, 0 means 100%
 * transparent, and 127 means 100% opaque.
 */
#define FONT_ALPHA 75

extern gdImagePtr image;
extern const char *font_file_or_name;
int annotateColor = -1;

extern void text_in_bbox(const char *text, bbox box, int color, double maxsize);
extern int _text_last_height;
extern double _text_last_sz;
extern int reverse_flag;

/**
 * @param Create CIDR annotation on gd hilbert image
 *
 * Calculate the bounding box, draw an outline of the box, then render the
 * text.
 */
void annotate_cidr(const char *cidr, const char *label, const char *sublabel) {
  bbox box = bbox_from_cidr(cidr);
  if (box.xmin < 0) {
    fprintf(stderr, "Warning: annotation %s is out of range for this image\n",
            cidr);
    return;
  }
  bbox_draw_outline(box, image, annotateColor);
  text_in_bbox(label, box, annotateColor, 128.0);
  if (sublabel) {
    bbox box2 = box;
    box2.ymin = (box.ymin + box.ymax) / 2 + (int)(_text_last_sz / 2) + 6;
    box2.ymax = box2.ymin + 24;
    if (0 == strcmp(sublabel, "prefix"))
      sublabel = cidr;
    text_in_bbox(sublabel, box2, annotateColor, 64.0);
  }
}

/**
 * @brief Read in annotation file
 *
 * Input is a text file with TAB-separated fields First field is a CIDR address
 * Second field is a text label
 */
void annotate_file(const char *fn) {
  char buf[512];
  FILE *fp = fopen(fn, "r");
  if (NULL == fp)
    err(1, "%s", fn);
  if (annotateColor < 0) {
    if (reverse_flag)
      annotateColor = gdImageColorAllocateAlpha(image, 0, 0, 0, FONT_ALPHA);
    else
      annotateColor =
          gdImageColorAllocateAlpha(image, 255, 255, 255, FONT_ALPHA);
  }
  if (!gdFTUseFontConfig(1))
    warnx("fontconfig not available");
  while (NULL != fgets(buf, 512, fp)) {
    char *cidr;
    char *label;
    char *sublabel = NULL;
    cidr = strtok(buf, "\t");
    if (NULL == cidr)
      continue;
    label = strtok(NULL, "\t\r\n");
    if (NULL == label)
      continue;
    sublabel = strtok(NULL, "\t\r\n");
    annotate_cidr(cidr, label, sublabel);
  }
  fclose(fp);
}
