#include "frame.h"

RawFrame::RawFrame(const int w, const int h): width(w), height(h) {}

PadFrame::PadFrame(const int w, const int h) {
  this->raw_width = w;
  this->raw_height = h;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = w % 16) > 0) ? w + 16 - pd : w;
  this->height = ((pd = h % 16) > 0) ? h + 16 - pd : h;
}

PadFrame::PadFrame(const RawFrame& rf) {
  this->raw_width = rf.width;
  this->raw_height = rf.height;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = rf.width % 16) > 0) ? rf.width + 16 - pd : rf.width;
  this->height = ((pd = rf.height % 16) > 0) ? rf.height + 16 - pd : rf.height;

  // Reserve the pixel vectors
  int pixels_per_unit = this->width * this->height;
  this->Y.reserve(pixels_per_unit);
  this->Cr.reserve(pixels_per_unit);
  this->Cb.reserve(pixels_per_unit);

  // Set up pointers for insertion
  auto y_ptr = rf.Y.begin();
  auto cr_ptr = rf.Cr.begin();
  auto cb_ptr = rf.Cb.begin();
  auto pad_y_ptr = this->Y.begin();
  auto pad_cr_ptr = this->Cr.begin();
  auto pad_cb_ptr = this->Cb.begin();

  // Insert into vectors
  for (int i = 0; i < rf.height; i++) {
    // Insert original pixels
    this->Y.insert(pad_y_ptr, y_ptr, y_ptr + rf.width);
    this->Cr.insert(pad_cr_ptr, cr_ptr, cr_ptr + rf.width);
    this->Cb.insert(pad_cb_ptr, cb_ptr, cb_ptr + rf.width);

    // Insert padding pixels (additional width)
    this->Y.insert(pad_y_ptr + rf.width, this->width - rf.width, 0);
    this->Cr.insert(pad_cr_ptr + rf.width, this->width - rf.width, 0);
    this->Cb.insert(pad_cb_ptr + rf.width, this->width - rf.width, 0);

    // Move pointers
    y_ptr += rf.width;
    cr_ptr += rf.width;
    cb_ptr += rf.width;
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }

  // Insert padding pixels (additional height)
  for (int i = rf.height; i < this->height; i++) {
    this->Y.insert(pad_y_ptr, this->width, 0);
    this->Cr.insert(pad_cr_ptr, this->width, 0);
    this->Cb.insert(pad_cb_ptr, this->width, 0);

    // Move pointers
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }
}


int Frame::get_neighbor_index(const int curr_index, const int neighbor_type) {
  int neighbor_index = 0;

  switch(neighbor_type) {
    case MB_NEIGHBOR_UL: // Upper left neighbor
      if (curr_index % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - this->nb_mb_cols - 1;
      break;

    case MB_NEIGHBOR_U: // Upper neighbor
      neighbor_index = curr_index - this->nb_mb_cols;
      break;

    case MB_NEIGHBOR_UR: // Upper right neighbor
      if ((curr_index + 1) % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - this->nb_mb_cols + 1;
      break;

    case MB_NEIGHBOR_L: // Left neighbor
      if (curr_index % this->nb_mb_cols == 0)
        neighbor_index = -1;
      else
        neighbor_index = curr_index - 1;
      break;

    default:
      neighbor_index = -1;
      break;
  }

  // If neighbor doesn't exist, return -1
  if (neighbor_index < 0)
    neighbor_index = -1;
  return neighbor_index;
}
