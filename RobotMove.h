/*****************************************************************************************************/

void PWM_init(double position);

void PWM_Update1(double rightADC, double leftADC, double midADC, int bcState);

void barCode_Scan(double barcode_val, int *code_counter, double *min_val, double *max_val);



/*****************************************************************************************************/