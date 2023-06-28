#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#define espacoEEPROM 1000

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

#define S_HOME 0
#define S_MENU 1
#define S_SET_AL 2
#define S_SET_HOUR 3
#define S_SET_QTD 4
#define S_ACAO 5
#define RELE 12

uint8_t estado = S_HOME; // estado atual
uint8_t posPoint = 0;    // estado inicial do menu
uint8_t al = 0;

#define BTN_MENU 0
#define BTN_EXIT 1
#define BTN_UP 2
#define BTN_DOWN 3

uint8_t button[4] = {8, 9, 10, 11};

uint8_t button_states[4];

uint8_t handleButton(int btn)
{
  uint8_t new_value = digitalRead(button[btn]);
  uint8_t result = button_states[btn] != new_value && new_value == 1;
  button_states[btn] = new_value;
  return result;
}

void epromWriteInt(int address, uint8_t value)
{
  byte hiByte = highByte(value);
  byte loByte = lowByte(value);

  EEPROM.write(address, hiByte);
  EEPROM.write(address + 1, loByte);
}

uint8_t epromReadInt(int address)
{
  byte hiByte = EEPROM.read(address);
  byte loByte = EEPROM.read(address + 1);
  return word(hiByte, loByte);
}

// variaveis definição de horario
uint8_t hour = 0;
uint8_t min = 0;
uint8_t qtd = 0;
uint8_t alarm[2][4] = {0, 0, 0, 1, 0, 0, 0, 1};
int salt = 0;

// hora modulo rtc.
uint8_t HOUR;
uint8_t MIN;
int porcao = 1;

// vairaveis de controle de tempo
unsigned long timeDispenser[2] = {0, 0};
unsigned long timeExitMenu = 0;
unsigned long controlePiscaTimer = 0;
unsigned long controleEstadoAcao = 0;

// variaveis formatação horario
String hourFormat = "";
String minFormat = "";
uint8_t st = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(button[BTN_MENU], INPUT_PULLUP);
  pinMode(button[BTN_EXIT], INPUT_PULLUP);
  pinMode(button[BTN_UP], INPUT_PULLUP);
  pinMode(button[BTN_DOWN], INPUT_PULLUP);

  button_states[0] = HIGH;
  button_states[1] = HIGH;
  button_states[2] = HIGH;
  button_states[3] = HIGH;

  // Limpeza da Memoria EEPROM(Executar apenas uma vez e dopois comentar o codigo
  // for(int n = 0; n < espacoEEPROM; n++){
  // EEPROM.write(n, 0);
  //}

  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      alarm[i][j] = epromReadInt(salt);
      salt += 2;
    }
  }

  lcd.begin(16, 2);

  if (!rtc.begin())
  {
    lcd.setCursor(0, 1);
    lcd.print("modulo on");
  }
  if (!rtc.isrunning())
  {                                    // SE RTC NÃO ESTIVER SENDO EXECUTADO, FAZ
    Serial.println("DS1307 rodando!"); // IMPRIME O TEXTO NO MONITOR SERIAL
    // REMOVA O COMENTÁRIO DE UMA DAS LINHAS ABAIXO PARA INSERIR AS INFORMAÇÕES ATUALIZADAS EM SEU RTC
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 15, 33, 15)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  printHome();
}

/**Inicio de função home**/
void printHome()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("<< Hello >>");
  printHour();
}

void printHour()
{
  String HOURstring = formatDate(HOUR);
  String MINstring = formatDate(MIN);
  lcd.setCursor(5, 1);
  lcd.print(HOURstring);
  lcd.print(":");
  lcd.print(MINstring);
}

void piscaTimer(int st)
{
  if (st == 0)
  {
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }
  if (st == 1)
  {
    lcd.setCursor(7, 1);
    lcd.print(":");
  }
}

void printMenu(int point = 0)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SET ALARM 0:");
  lcd.setCursor(0, 1);
  lcd.print("SET ALARM 1:");

  lcd.setCursor(14, point);
  lcd.print("<");
}

void getArray(int al = 0)
{
  hour = alarm[al][0];
  min = alarm[al][1];
  qtd = alarm[al][2];

  hourFormat = formatDate(hour);
  minFormat = formatDate(min);
}

String formatDate(int num)
{
  String numString = "";
  numString = num < 10 ? "0" + String(num) : String(num);
  return numString;
}

void printSetAlarm(int al = 0, int point = 4)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AL_");
  lcd.setCursor(3, 0);
  lcd.print(al);
  lcd.setCursor(9, 0);
  lcd.print(hourFormat);
  lcd.print(":");
  lcd.print(minFormat);
  lcd.setCursor(0, 1);
  lcd.print("PORCOES:");
  lcd.setCursor(12, 1);
  lcd.print(qtd);

  if (point != 4)
  {
    lcd.setCursor(15, point);
    lcd.print("<");
  }
}

/*void printSetHour(int point = 6){
  hourFormat = formatDate(hour);
  minFormat = formatDate(min);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HORA: ");
  lcd.print(hourFormat);
  lcd.print(":");
  lcd.print(minFormat);

  lcd.setCursor(point,1);
  lcd.print("^^");
}*/

void printSetHour(int point = 4)
{
  hourFormat = formatDate(hour);
  minFormat = formatDate(min);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HR: ");
  lcd.print(hourFormat);
  lcd.print(":");
  lcd.print(minFormat);
  lcd.setCursor(11, 0);
  lcd.print("P: 0");
  lcd.print(qtd);
  lcd.setCursor(point, 1);
  lcd.print("^^");
}

void printSetQtd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RACAO: ");
  lcd.print(qtd);
}

void saveConfig(int al, int set)
{
  alarm[al][0] = hour;
  alarm[al][1] = min;
  alarm[al][2] = qtd;

  for (int i = 0; i <= 2; i++)
  {
    epromWriteInt(set, alarm[al][i]);
    set += 2;
  }
}

// função para ler e salvar valores na eeprom

// função tela alarm

void telaAl()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alimentando");
}

void setTimeExitMenu()
{
  timeExitMenu = millis();
}

void checkTimeExitMenu()
{
  if (millis() - timeExitMenu >= 50000)
  {
    if (estado == S_SET_HOUR || estado == S_SET_AL)
    {
      estado = S_HOME;
      int set = al == 0 ? 0 : 6;
      saveConfig(al, set);
      posPoint = 0;
      al = 0;
      printHome();
      // break;
    }
    else
    {
      estado = S_HOME;
      posPoint = 0;
      al = 0;
      printHome();
    }
  }
}

void printTestAcao()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("FUNCIONANDO");
}

// lOP PRINCIPAL = = = = = = = = = = = = = = = = = = = = = =
void loop()
{

  DateTime now = rtc.now();
  HOUR = now.hour();
  MIN = now.minute();

  switch (estado)
  {
  case S_HOME: /**Inicio Home**/
    if (handleButton(BTN_MENU))
    {
      estado = S_MENU;
      printMenu(0);
      setTimeExitMenu();
      break;
    }

    if (millis() - controlePiscaTimer >= 500)
    {
      controlePiscaTimer = millis();
      st = st == 1 ? 0 : 1;
      printHour();
      piscaTimer(st);
    }

    for (byte i = 0; i <= 1; i++)
    {
      if (HOUR == alarm[i][0] && MIN == alarm[i][1] && alarm[i][3] == 1)
      {
        timeDispenser[i] = millis();
        controleEstadoAcao = millis();
        printTestAcao();
        porcao = alarm[i][2] > 0 ? alarm[i][2] : 1;
        estado = S_ACAO;
        break;
        /*while(millis() - timeDispenser[i] <=100000){
          digitalWrite(RELE, HIGH);
          alarm[i][3] = 0;
          telaAl();
        }
        digitalWrite(RELE, LOW);
        printHome();*/
      }
    }

    for (byte i = 0; i <= 1; i++)
    {
      if (alarm[i][3] == 0)
      {
        if (millis() - timeDispenser[i] >= 60000)
        {
          alarm[i][3] = 1;
        }
      }
    }
    if (handleButton(BTN_DOWN))
    {
      controleEstadoAcao = millis();
      estado = S_ACAO;
      printTestAcao();
      break;
    }

    break; /**fim Home**/

  case S_MENU: /*Inicio menu*/

    if (handleButton(BTN_UP))
    {
      printMenu(0);
      posPoint = 0;
    }

    if (handleButton(BTN_DOWN))
    {
      printMenu(1);
      posPoint = 1;
    }

    if (handleButton(BTN_MENU))
    {
      estado = S_SET_AL;
      al = posPoint;
      getArray(al);
      printSetAlarm(al, 0);
      setTimeExitMenu();
      posPoint = S_SET_HOUR;
      break;
    }

    if (handleButton(BTN_EXIT))
    {
      setTimeExitMenu();
      estado = S_HOME;
      al = 0;
      posPoint = 0;
      printHome();
      break;
    }
    checkTimeExitMenu();
    break; /*Fim menu*/

  case S_SET_AL: /*Inicio do menu set Alarme*/
    if (handleButton(BTN_UP))
    {
      printSetAlarm(al, 0);
      posPoint = S_SET_HOUR;
    }

    if (handleButton(BTN_DOWN))
    {
      printSetAlarm(al, 1);
      posPoint = S_SET_QTD;
    }

    if (handleButton(BTN_MENU))
    {
      setTimeExitMenu();
      estado = S_SET_HOUR;
      posPoint = posPoint == S_SET_HOUR ? 4 : 14;
      printSetHour(posPoint);
      break;
    }

    if (handleButton(BTN_EXIT))
    {
      setTimeExitMenu();
      estado = S_MENU;
      posPoint = 0;
      printMenu(0);
      break;
    }

    checkTimeExitMenu();
    break; /*finale menu set Alarme*/

  case S_SET_HOUR:
    if (handleButton(BTN_MENU))
    {
      //  posPoint = posPoint!= 7?7:4;

      if (posPoint == 4)
      {
        posPoint = 7;
      }
      else if (posPoint == 7)
      {
        posPoint = 14;
      }
      else
      {
        posPoint = 4;
      }

      printSetHour(posPoint);
    }

    if (handleButton(BTN_UP))
    {
      if (posPoint == 4)
      {
        hour = hour < 23 ? hour += 1 : 0;
        printSetHour(posPoint);
      }
      if (posPoint == 7)
      {
        min = min < 59 ? min += 1 : 0;
        printSetHour(posPoint);
      }
      if (posPoint == 14)
      {
        qtd = qtd < 5 ? qtd += 1 : 1;
        printSetHour(posPoint);
      }
    }

    if (handleButton(BTN_DOWN))
    {
      if (posPoint == 4)
      {
        hour = hour <= 0 ? 23 : hour -= 1;
        printSetHour(posPoint);
      }
      if (posPoint == 7)
      {
        min = min <= 0 ? 59 : min -= 1;
        printSetHour(posPoint);
      }
      if (posPoint == 14)
      {
        qtd = qtd > 1 ? qtd -= 1 : 5;
        printSetHour(posPoint);
      }
    }

    if (handleButton(BTN_EXIT))
    {
      setTimeExitMenu();
      estado = S_SET_AL;
      int set = al == 0 ? 0 : 6;
      saveConfig(al, set);
      posPoint = S_SET_HOUR;
      printSetAlarm(al, 0);
      break;
    }
    checkTimeExitMenu();
    break;

  case S_ACAO:

    if (millis() - controleEstadoAcao <= porcao * 1000)
    {
      digitalWrite(RELE, HIGH);
      break;
    }
    else
    {
      digitalWrite(RELE, LOW);
      porcao = 1;
      estado = S_HOME;
      printHome();
      break;
    }

    break;

    /*case S_SET_QTD:
   if(handleButton(BTN_UP)){
      qtd = qtd < 10 ? qtd += 1: 0;
      printSetQtd();
    }

    if(handleButton(BTN_DOWN)){
      qtd = qtd > 0 ? qtd -= 1: 10;
      printSetQtd();
    }

    if(handleButton(BTN_EXIT)){
      estado = S_SET_AL;
      posPoint = 0;
      printSetAlarm(al, 0);
      break;
    }*/

    break;
  }
}