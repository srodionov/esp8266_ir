//char              template_IR[]   = "header|command|post_data|ptrail|gap|command|ptrail|repeat_gap|header|command|post_data|ptrail|gap|command|ptrail";
char              template_IR[]   = "header|command|post_data|ptrail|gap|command|ptrail";

int               frequency       = 38;
unsigned char     bits            = 32;
unsigned short    header[2]       = {9000, 4500};
unsigned short    one[2]          = {700, 1600};
unsigned short    zero[2]         = {700, 500};
unsigned short    ptrail          = 700;
unsigned short    gap             = 19900;
unsigned short    post_data_bits  = 3;
unsigned long     post_data       = 0x2;
unsigned short    repeat_gap      = 39900;
unsigned char     min_repeat      = 0;

void addSignal(unsigned int *arr, unsigned short value) {
  int i = 0;
  while (arr[i] != 0) {i++;}
  arr[i] = value;
}

void addPulse(unsigned int *arr, unsigned short pulse, unsigned short space) {
  addSignal(arr, pulse);
  addSignal(arr, space);
}

void addCommand(unsigned int *arr, unsigned long cmd, unsigned char cmd_bits) {
  unsigned long bitMask = 0;
   
  for (int i = cmd_bits - 1; i >= 0; i--)
  {
    bitMask = (1 << i);  
    if ((cmd & bitMask) == bitMask) {
      addPulse(arr, one[0], one[1]);
      //Serial.print("1");
    }else {
      addPulse(arr, zero[0], zero[1]);
      //Serial.print("0");
    }
  }
  //Serial.println("");
}

unsigned short getArraySize(char *iTemplateIR) {
  unsigned short res = 0;
  char copyTemplate[strlen(iTemplateIR)];

  strcpy(copyTemplate, iTemplateIR);
  char *pch = strtok(copyTemplate,"|");
  while (pch != NULL) 
  {
    if (strcmp(pch, "header")==0) res = res+2;
    else if (strcmp(pch, "command")==0) res = res+bits*2; 
    else if (strcmp(pch, "post_data")==0) res = res+post_data_bits*2; 
    else if ((strcmp(pch, "ptrail")==0) || (strcmp(pch, "gap")==0) || (strcmp(pch, "repeat_gap")==0)) res = res+1;        
    pch = strtok(NULL, "|");
  }
  return res;
}
 
void sendIR(char *message) {  
  unsigned short rawSize = 0;
   
  char copyTemplate[strlen(template_IR)];
  strcpy(copyTemplate, template_IR);  
    
  rawSize = getArraySize(template_IR);
  unsigned int RAWsignal[rawSize];
  for (int i=0;i<rawSize;i++) RAWsignal[i] = 0;

  unsigned long commands[8];
  int iCommand = 0;
  char *pch = strtok (message," ,");
  while (pch != NULL) 
  {
    commands[iCommand] = strtoul(pch, NULL, 16);;
    iCommand++;    
    pch = strtok (NULL, " ,");  
  }
  
  iCommand=0;
  pch = strtok (copyTemplate,"|");
  while (pch != NULL) 
  {    
    if (strcmp(pch, "header")==0) addPulse(RAWsignal, header[0], header[1]);
    else if (strcmp(pch, "command")==0){      
      addCommand(RAWsignal, commands[iCommand], bits);
      iCommand++;
    }
    else if (strcmp(pch, "post_data")==0) addCommand(RAWsignal, post_data, post_data_bits);
    else if (strcmp(pch, "ptrail")==0) addSignal(RAWsignal, ptrail);
    else if (strcmp(pch, "gap")==0) addSignal(RAWsignal, gap);
    else if (strcmp(pch, "repeat_gap")==0) addSignal(RAWsignal, repeat_gap);
              
    pch = strtok (NULL, "|");
  }
  
  //Serial.println("RAWsignal: ");
  //for(int i=0;i<sizeof(RAWsignal)/sizeof(RAWsignal[0]);i++){
    //Serial.println(RAWsignal[i]);
    //Serial.print(" ");
  //}
  irsend.sendRaw(RAWsignal, sizeof(RAWsignal)/sizeof(RAWsignal[0]), frequency);
}
