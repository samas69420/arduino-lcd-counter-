#include <Wire.h>                                                                        // libreria per i2c
#include <LiquidCrystal_I2C.h>                                                           // libreria per schermolcd

#define I2C_ADDR 0x27                                                                    // indirizzo i2c schermo

#define BACKLIGHT_PIN 3                                                                  ////
#define En_pin 2                                                                         ///
#define Rw_pin 1                                                                         ///
#define Rs_pin 0                                                                         ///  pin per lo schermo
#define D4_pin 4                                                                         ///
#define D5_pin 5                                                                         ///
#define D6_pin 6                                                                         ///
#define D7_pin 7                                                                         ////

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);        // creo l'oggetto di tipo LiquidCrystal_I2C

#define button 5                                                                         // pin di input per il pulsante
#define deltap 300                                                                       // deltap è il massimo intervallo di tempo entro cui due click consecutivi vengono considerati doppio click e non due click separati
#define deltaplungo 1500                                                                 // tempo oltre il quale la pressione continua del tasto diventa "click lungo"
unsigned long time0 = 0;                                                                 // variabile per prendere il tempo iniziale (la userò in leggipulsante)
unsigned long t = 0;                                                                     // variabile generica per prendere il tempo

#define screenfreq 500                                                                   // variabile di refresh dello schermo (e in questo caso anche delle animazioni)

char testo_scorrevole[] = "testo scorrevole ";                                           // testo che appare nella prima riga dello schermo e scorre
const int list_dim = 16;                                                                 // dimensione dell' array che vedrò a schermo (in questo caso la larghezza dello schermo lcd)

const int testo_scorrevole_dim = sizeof(testo_scorrevole)-1;                             // numero di caratteri da cui è composta testo_scorrevole
char list[list_dim];                                                                     // array effettivo che vedrò a schermo e che verrà riscritto a ogni loop per creare l'effetto di scorrimento
int i=0;                                                                                 // indice che mi terrà conto di dove sono arrivato nella lettura del testo da far scorrere, servirà per riscrivere l'array da stampare

int stato = 0;                                                                           // variabile che avrà lo stato del pulsante e verrà usata per scegliere la riposta del programma al click
int contatore_kill = 0;                                                                  // contatore che si vede a schermo e che aumenta con il pulsante


// legge il pulsante e ritorna 1(click) 2(doppio click) 3(click lungo)
int leggipulsante(){
      if(digitalRead(button) == 0){return 0;}                                            // se il pulsante non è schiacciato ritorno 0 ed esco subito
      time0=millis();                                                                    // prendo il tempo iniziale
      while(digitalRead(button)==1){delay(60);}                                          // finchè il pulsante rimane premuto aspetto 60ms per stabilizzare la lettura e basta 
      if(millis()-time0>=deltaplungo){return 3;}                                         // appena il pulsante viene rilasciato vedo per quanto tempo è stato tenuto premuto, se la cosa è durata più di "deltaplungo" ritorno 3
      else if(millis()-time0<deltaplungo){                                               // se invece la pressione è durata meno di deltaplungo vado avanti
        time0 = millis();                                                                // prendo ancora il tempo iniziale
        while(millis()-time0<deltap){                                                    // aspetto che da quando ho preso il tempo iniziale passino deltap ms
          if(digitalRead(button) == 1){                                                  ////
            while(digitalRead(button) == 1){delay(60);}                                  /// se mentre aspetto i deltap ms arriva un altro click allora fino a quando il pulsante rimane premuto aspetto 60ms per stabilizzare poi ritorno 2
            return 2;                                                                    ////
          }
        }
        return 1;                                                                        // se passano i deltap ms e non c'è stato nessun click allora ho un click singolo e ritorno 1
      }
}


// funzioni che verranno chiamate come risposta ai click

void clicksingolo(){
  contatore_kill++;
}
void doppioclick(){
  contatore_kill--;
}
void clicklungo(){
  contatore_kill = 0;
}


// prende in input un intero n e stampa sullo schermo lcd tutta la lista globale "list" al primo rigo e "KIL... n" al secondo
void scrivi_schermo(int n){
  lcd.setCursor(0,0);                                                                   // posiziono il cursore all'inizio della prima riga
  for(int i=0; i<list_dim-1;i++){                                                       ////
    lcd.print(list[i]);                                                                 /// scrivo "list" sulla prima riga dello schermo
  }                                                                                     ////
  lcd.setCursor(0,1);                                                                   // posiziono il cursore all'inizio della seconda riga
  lcd.print("KILLZ COUNTER:");                                                          ////
  lcd.print(n);                                                                         ///
  lcd.print(" ");                                                                       /// scrivo KILLZ COUNTER: n sulla seconda riga dello schermo con il vaore di n in quel momento
  t=millis();                                                                           ////
}


// prende per indirizzo un array che nel metodo sarà "text", la sua dimensione e due interi e scrive l'array grobale "list" leggendo da indice "a" a indice "b" 
void list_from_a_to_b(char* text, int text_dim,int a, int b){
  if (b > text_dim){                                                                    // se l'indice b è maggiore della dimensione del text allora faccio due cose
    for(int k=a; k<text_dim; k++){                                                      ////
      list[k-a]=text[k];                                                                /// prima: carico la prima parte di list con gli elementi nell' intervallo che va dall'indice "a" alla fine di text, qualcosa tipo: [a,text_dim[
    }                                                                                   ////
    for(int t=text_dim-a; t<list_dim; t++){                                             ////
       list[t]=text[t-text_dim+a];                                                      /// seconda: carico l'ultima parte di list con il numero mancante di elementi prendendoli a partire dall'inizio di text, tipo: [0, b-text_dim[
    }                                                                                   ////
  }
  if (b <= text_dim){                                                                   ////
    for(int i=a;i<b;i++){                                                               ///
      list[i-a]=text[i];                                                                /// se l'indice b è minore o uguale a text_dim allora tutto l'intervallo [a,b] sarà in text quindi lo carico in list senza troppi problemi
    }                                                                                   ///
  }                                                                                     ////
}


// prende per indirizzo un array che nel metodo sarà "testo_calcolato" e la sua dimensione poi li usa per aggiornare lo schermo 
void screen_loop(char* testo_calcolato, int testo_calcolato_dim ){                      //
   list_from_a_to_b(testo_calcolato,testo_calcolato_dim,i, i+list_dim);                 // da testo_calcolato genero la lista che uscirà a schermo usando i e i+list_dim come indici "a" e "b" 
   scrivi_schermo(contatore_kill);                                                      // dopo aver generato la lista la stampo con anche il contatore delle kill
   i++;                                                                                 // incremento l'indice che così crea lo scorrimento
   if(i == testo_calcolato_dim) i=0;                                                    // se l'indice iniziale ha raggiunto la fine di tutto l'array che doveva scorrere lo riposto a 0
}


// prende per indirizzo un array che nel metodo sarà "testo" e la sua dimensione poi lo carica ripetendo più volte "testo_scorrevole"
void moltiplica_testo_corto(char* testo, int testo_dim){
  int ind=0;                                                                            // dichiaro un indice a 0 che userò per prendere i caratteri da testo_scorrevole
  for(int i=0;i<testo_dim;i++){                                                         ////
    testo[i]=testo_scorrevole[ind];                                                     /// carico l'array testo con testo_scorrevole fino a riempirlo tutto, quando finisce testo_scorrevole riporto ind a 0 e ricomincio
    ind++;                                                                              ///
    if(ind==testo_scorrevole_dim)ind=0;                                                 ////
  }
}





void setup() {
  lcd.begin(16,2);                                                                                             ////
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);                                                                 /// istruzioni per inizializzare lo schermo
  lcd.setBacklight(HIGH);                                                                                      ///
  lcd.home();                                                                                                  ////
  pinMode(button, INPUT);                                                                                      // metto in input il pin del bottone

  if(testo_scorrevole_dim<list_dim){                                                                           // (1) - se il testo scorrevole che gli ho dato da codice è più lungo dell'array list:
      const int testo_dim = testo_scorrevole_dim*(int)ceil(((double)list_dim)/testo_scorrevole_dim);           //       calcolo la dimensione del testo 
      char *testo = malloc(sizeof(char)*testo_dim);                                                            //       creo l'array testo o meglio alloco la memoria che mi serve per l'array con malloc dopo aver calcolato quanto mi serve
      moltiplica_testo_corto(testo,testo_dim);                                                                 //       uso il metodo moltiplica_testo_corto per caricare l'array che ho appena allocato
      main_loop(testo,testo_dim);                                                                              //       lancio il loop parametrizzato passandogli l'array caricato e rimango qui 
  }
  if(testo_scorrevole_dim>=list_dim){                                                                          // (2) - se il testo scorrevole che gli ho dato da codice è più lungo dell'array list:
      main_loop(testo_scorrevole,testo_scorrevole_dim);                                                        //       non ho particolari problemi quindi lancio subito il loop parametrizzato direttamente con testo_scorrevole e rimango qui
  }
 
}





// funzione che uso al posto del loop normale, è un loop infinito ma è anche parametrico e viene chiamato nel setup così posso distinguere i casi per il testo_scorrevole
void main_loop(char* text, int text_dim){
  while(true){
    stato=leggipulsante();                                                                                     // prendo lo stato del pulsante
    if(stato == 1 ){clicksingolo();}                                                                           ////
    if(stato == 2 ){doppioclick();}                                                                            /// eseguo la funzione corrispondente al click che ho rilevato e memorizzato nella variabile stato
    if(stato == 3 ){clicklungo();}                                                                             ////
    if(millis()-t>=screenfreq)(screen_loop(text,text_dim));                                                    // apetto che passino screenfreq ms poi scrivo lo schermo 
  }
}


// loop normale che in questo caso non mi serve e non viene neanche eseguito
void loop(){}

/*      BUG DA RISOLVERE:
 *     - mentre il pulsante è schiacciato il testo smette di scorrere (devo rompere i while in leggipulsante)
 */
