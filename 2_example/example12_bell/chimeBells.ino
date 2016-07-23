/*******************************************************************************
チャイム
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

int chimeBells(int output, int count) {
	int t;
	if(count<=0) return 0;
	if(count%10==0){
		if(!((count/10)%2)){
			tone(output,NOTE_CS6,800);
			for(t=0;t<8;t++) delay(100);
			noTone(output);
		}else{
			tone(output,NOTE_A5,800);
			for(t=0;t<8;t++) delay(100);
			noTone(output);
		}
	}
	delay(100);
	count--;
	if(count<0) count=0;
	return(count);
}
