#include "qdct.h"

inline void mat_mul(const int a[4][4], const int b[4][4], int c[4][4]) {
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			c[i][j] = 0;
      for(int k = 0; k < 4; k++)
        c[i][j] += a[i][k] * b[k][j];
		}
	}
}

/* Core transformation on 4x4 block
 *
 * By formula:
 * (0, 0), (2, 0), (0, 2), (2, 2) mf = mat_MF[QP % 6][0]
 * (1, 1), (3, 1), (1, 3), (3, 3) mf = mat_MF[QP % 6][1]
 * other positions                mf = mat_MF[QP % 6][2]
 */
void core_transform(const int mat_x[4][4], int mat_z[4][4]) {
	// Compute core matrix
  int mat_temp[4][4];
	mat_mul(mat_Cf, mat_x, mat_z);
  mat_mul(mat_z, mat_Cf_T, mat_temp);

  // Set up transform arguments
  int qbits = 15 + floor(QP / 6);
  int f = (int)(pow(2.0, qbits) / 3.0);
  int mf, k;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      mf = mat_MF[QP % 6][k];
      mat_z[i][j] = (abs(mat_temp[i][j]) * mf + f) >> qbits;

      if (mat_temp[i][j] < 0)
        mat_z[i][j] = -mat_z[i][j];
    }
  }
}

/* Inversed core transformation on 4x4 block
 */
void inv_core_transform(const int mat_x[4][4], int mat_z[4][4]) {
  int t = floor(QP / 6);
  int f = (int)pow(2.0, t);
  int k, v;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == 0 || i == 2) && (j == 0 || j == 2))
        k = 0;
      else if ((i == 1 || i == 3) && (j == 1 || j == 3))
        k = 1;
      else
        k = 2;

      v = mat_V[QP % 6][k];
      mat_z[i][j] = mat_x[i][j] * v * f;
    }
  }

  // Inverse from core matrix
  int mat_temp[4][4];
	mat_mul(mat_Ci, mat_z, mat_temp);
  mat_mul(mat_temp, mat_Ci_T, mat_z);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++)
      mat_z[i][j] = int(mat_z[i][j] / 256.0 + 0.5);
  }
}

/* Quantized Discrete Cosine Transform
 *
 * The interface of QDCT, apply on 16x16 block
 */
void qDCT(Block16x16& block) {
  int mat_x[4][4], mat_z[4][4];

  // Apply 4x4 core transform 16 times on 16x16 block
  for (int i = 0; i < 256; i += 64) {
    for (int j = 0; j < 16; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*16+x];
      }

      // Apply 4x4 core transform
      core_transform(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*16+x] = mat_z[y][x];
      }
    }
  }
}

/* Quantized Discrete Cosine Transform
 *
 * The interface of QDCT, apply on 8x8 block
 */
void qDCT(Block8x8& block) {
  int mat_x[4][4], mat_z[4][4];

  // Apply 4x4 core transform 4 times on 8x8 block
  for (int i = 0; i < 64; i += 32) {
    for (int j = 0; j < 8; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*8+x];
      }

      // Apply 4x4 core transform
      core_transform(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*8+x] = mat_z[y][x];
      }
    }
  }
}

/* Inverse Quantized Discrete Cosine Transform
 *
 * The interface of inversed QDCT, apply on 16x16 block
 */
void inv_qDCT(Block16x16& block) {
  int mat_x[4][4], mat_z[4][4];

  // Apply 4x4 core transform 16 times on 16x16 block
  for (int i = 0; i < 256; i += 64) {
    for (int j = 0; j < 16; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*16+x];
      }

      // Apply 4x4 core transform
      inv_core_transform(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*16+x] = mat_z[y][x];
      }
    }
  }
}

/* Inverse Quantized Discrete Cosine Transform
 *
 * The interface of inversed QDCT, apply on 8x8 block
 */
void inv_qDCT(Block8x8& block) {
  int mat_x[4][4], mat_z[4][4];

  // Apply 4x4 core transform 4 times on 8x8 block
  for (int i = 0; i < 64; i += 32) {
    for (int j = 0; j < 8; j += 4) {
      // Copy into 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          mat_x[y][x] = block[i+j+y*8+x];
      }

      // Apply 4x4 core transform
      inv_core_transform(mat_x, mat_z);

      // Write back from 4x4 matrix
      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++)
          block[i+j+y*8+x] = mat_z[y][x];
      }
    }
  }
}