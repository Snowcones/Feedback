

void genNoiseFloat(int width, int height, int initialHorDiv, int initialVertDiv, int octaves, float persist, float* store, float* octaveNoise, int interp);
void genNoiseShort(int width, int height, int octaves, float persist, short* store, short* octaveNoise, int interp);

void genOctaveFloat(int totalHorDiv, int totalVertDiv, float* store, int width, int height, int interp);
void genOctaveShort(int totalHorDiv, int totalVertDiv, short* store, int width, int height, int interp);

void addArrToArrFloat(float* arr1, float* arr2, float weight, int len);
void addArrToArrShort(int* arr1, int* arr2, int len);

float dotProd(float x1, float y1, float x2, float y2);