
#include "RFID_command.h"
#include "Print.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <dos.h>

/*
void UHF_RFID::Sendcommand(UBYTE com_nub)
{
  UBYTE b = 0;
  while (RFID_cmdnub[com_nub][b] != 0x7E)
  {
    Serial2.write(RFID_cmdnub[com_nub][b]);
    b++;
  }
  Serial2.write(0x7E);
  Serial2.write("\n\r");
}

void UHF_RFID::Send_the_modified_command()
{
  UBYTE b = 0;//Send instructions å�‘é€�æŒ‡ä»¤

  while (DATA_Interim_order[b] != 0x7E)
  {
    Serial2.write(DATA_Interim_order[b]);
    b++;
  }
  Serial2.write(0x7E);
  Serial2.write("\n");

}

void UHF_RFID::Readcallback()
{
  while (Serial2.available())
  {
    Delay(2);
    DATA_I[DATA_I_NUB] = Serial2.read();
    if (_debug == 1)
    {
      Serial.print(DATA_Str_Serial);
    }
    DATA_I_NUB++;
  }
}
*/
void UHF_RFID::clean_data()
{
  DATA_Str_Serial = "";
  DATA_Str_M5led = "";
  DATA_I_NUB = 0;
  DATA_Interim_b = 0;
  for (UDOUBLE i = 0; i < 4096; i++)
  {
    DATA_I[i] = 0x00;
  }
  for (UDOUBLE i = 0; i < 40; i++)
  {
    DATA_Interim_order[i] = 0x00;
  }
}


UBYTE UHF_RFID::Return_to_convert(UBYTE mod)
{
  DATA_Str_M5led = "";
  switch (mod)
  {
    case 0:                                 //Convert to a string è½¬æ�¢ä¸ºå­—ç¬¦ä¸²
      for (int b = 0; b <= DATA_I_NUB; b++)
      {
        DATA_Str_M5led += char(DATA_I[b]);
      }
      break;
    case 1:                       //Convert to a HEX string è½¬æ�¢ä¸ºHEXå­—ç¬¦ä¸²
      for (int b = 0; b <= DATA_I_NUB; b++)
      {
        if (DATA_I[b] == 0x00)
        {
          DATA_Str_M5led += "00";
        }
        else if (DATA_I[b] < 0x10)
        {
          DATA_Str_M5led += "0";
          DATA_Str_M5led += String(DATA_I[b], HEX);
        }
        else
        {
          DATA_Str_M5led += String(DATA_I[b], HEX);
        }
      }
      break;
    default:
      return 0;
      break;
  }
  return 1;

}

UDOUBLE UHF_RFID::String_to_command_frame(String str_epc) //EPC string to command frame EPCå­—ç¬¦ä¸²è½¬å‘½ä»¤å¸§
{

  UDOUBLE b;

  for (UBYTE c = 0; c < 8; c++)
  {
    switch (char(str_epc[c]))
    {
      case '0':
        b = b * 0x10 + 0x0;
        break;
      case '1':
        b = b * 0x10 + 0x1;
        break;
      case '2':
        b = b * 0x10 + 0x2;
        break;
      case '3':
        b = b * 0x10 + 0x3;
        break;
      case '4':
        b = b * 0x10 + 0x4;
        break;
      case '5':
        b = b * 0x10 + 0x5;
        break;
      case '6':
        b = b * 0x10 + 0x6;
        break;
      case '7':
        b = b * 0x10 + 0x7;
        break;
      case '8':
        b = b * 0x10 + 0x8;
        break;
      case '9':
        b = b * 0x10 + 0x9;
        break;
      case 'a':
      case 'A':
        b = b * 0x10 + 0xa;
        break;
      case 'b':
      case 'B':
        b = b * 0x10 + 0xb;
        break;
      case 'c':
      case 'C':
        b = b * 0x10 + 0xc;
        break;
      case 'd':
      case 'D':
        b = b * 0x10 + 0xd;
        break;
      case 'e':
      case 'E':
        b = b * 0x10 + 0xe;
        break;
      case 'f':
      case 'F':
        b = b * 0x10 + 0xf;
        break;
    }
  }

  return b;

}


void UHF_RFID::Warningmessage(UBYTE nub)
{
  switch (nub)
  {
    case 0x17:
      DATA_Str_Serial = "Error instruction code in command frame.";
      //å‘½ä»¤å¸§ä¸­æŒ‡ä»¤ä»£ç �é”™è¯¯ã€‚
      break;
    case 0x20:
      DATA_Str_Serial = "Frequency-hopping search channel timeout.All channels are occupied during this time.";
      //è·³é¢‘æ�œç´¢ä¿¡é�“è¶…æ—¶ã€‚æ‰€æœ‰ä¿¡é�“åœ¨è¿™æ®µæ—¶é—´å†…éƒ½è¢«å� ç”¨ã€‚
      break;
    case 0x15:
      DATA_Str_Serial = "The polling operation failed.No tag is returned or CRC error is returned for data.";
      //è½®è¯¢æ“�ä½œå¤±è´¥ã€‚æ²¡æœ‰æ ‡ç­¾è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0x16:
      DATA_Str_Serial = "Failed to access the label, it is possible that the access password password is wrong";
      //è®¿é—®æ ‡ç­¾å¤±è´¥ï¼Œæœ‰å�¯èƒ½æ˜¯è®¿é—®å¯†ç � password ä¸�å¯¹
      break;
    case 0x09:
      DATA_Str_Serial = "Failed to read the label data save area.The tag does not return or returns a CRC error in the data" ;
      //è¯»æ ‡ç­¾æ•°æ�®å­˜æ•°åŒºå¤±è´¥ã€‚æ ‡ç­¾æ²¡æœ‰è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯
      break;
    case 0xA0:
      DATA_Str_Serial = "Error reading label data store.The returned Code is 0xA0 bit or Error Code To the.";
      //è¯»æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºé”™è¯¯ã€‚è¿”å›žçš„ä»£ç �ç”± 0xA0 ä½�æˆ–  Error Code å¾—åˆ°ã€‚
      break;
    case 0x10:
      DATA_Str_Serial = "Write label data save failed.The tag does not return or returns a CRC error in the dataBy mistake.";
      //å†™æ ‡ç­¾æ•°æ�®å­˜æ•°åŒºå¤±è´¥ã€‚æ ‡ç­¾æ²¡æœ‰è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0xB0:
      DATA_Str_Serial = "Error writing label data store.The returned Code is given by the 0xB0 bit or Error Code To the.";
      //å†™æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºé”™è¯¯ã€‚è¿”å›žçš„ä»£ç �ç”± 0xB0 ä½�æˆ–  Error Code å¾—åˆ°ã€‚
      break;
    case 0x13:
      DATA_Str_Serial = "Failed to lock label data save area.The tag does not return or returns a CRC error in the data By mistake.";
      //é”�å®šæ ‡ç­¾æ•°æ�®å­˜æ•°åŒºå¤±è´¥ã€‚æ ‡ç­¾æ²¡æœ‰è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0xC0:
      DATA_Str_Serial = "Error locking label data store.The Code returned is 0xC0 bit or Error Code Get it.";
      //é”�å®šæ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºé”™è¯¯ã€‚è¿”å›žçš„ä»£ç �ç”± 0xC0 ä½�æˆ–  Error Codeå¾—åˆ°ã€‚
      break;
    case 0x12:
      DATA_Str_Serial = "Inactivate tag failed.The tag does not return or returns a CRC error in the data.";
      //ç�­æ´»æ ‡ç­¾å¤±è´¥ã€‚æ ‡ç­¾æ²¡æœ‰è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0xD0:
      DATA_Str_Serial = "Inactivated tag error.The returned Code is given by the 0xC0 bit or Error Code.";
      //ç�­æ´»æ ‡ç­¾é”™è¯¯ã€‚è¿”å›žçš„ä»£ç �ç”± 0xC0 ä½�æˆ– Error Code å¾—åˆ°ã€‚
      break;
    case 0x14:
      DATA_Str_Serial = "BlockPermalock execution failed.The tag does not return or returns a CRC check of the data Error.";
      //BlockPermalock æ‰§è¡Œå¤±è´¥ã€‚æ ‡ç­¾æ²¡æœ‰è¿”å›žæˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0xE0:
      DATA_Str_Serial = "BlockPermalock errors.";
      //BlockPermalock é”™è¯¯ã€‚
      break;
    case 0x1A:
      DATA_Str_Serial = "The changeConfig directive failed. The tag does not return data or a return number Error check against CRC.";
      //ChangeConfig æŒ‡ä»¤å¤±è´¥ï¼Œæ ‡ç­¾æ²¡æœ‰è¿”å›žæ•°æ�®æˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0x2A:
      DATA_Str_Serial = "The ReadProtect instruction failed, the label does not return data or returns data CRC check error.";
      //ReadProtect æŒ‡ä»¤å¤±è´¥ï¼Œæ ‡ç­¾æ²¡æœ‰è¿”å›žæ•°æ�®æˆ–è€…è¿”å›žæ•°æ�®CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0x2B:
      DATA_Str_Serial = "Reset ReadProtect instruction failed, the label did not return data or returnBack data CRC check error.";
      //Reset ReadProtect æŒ‡ä»¤å¤±è´¥ï¼Œæ ‡ç­¾æ²¡æœ‰è¿”å›žæ•°æ�®æˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0x1B:
      DATA_Str_Serial = "The CHANGE EAS directive failed, and the label did not return data or returned data CRC check error.";
      //Change EAS æŒ‡ä»¤å¤±è´¥ï¼Œæ ‡ç­¾æ²¡æœ‰è¿”å›žæ•°æ�®æˆ–è€…è¿”å›žæ•°æ�®CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0x1D:
      DATA_Str_Serial = "The EAS_ALARM instruction failed with no label returning the correct ALARM CODE.";
      //EAS_Alarm æŒ‡ä»¤å¤±è´¥ï¼Œæ²¡æœ‰æ ‡ç­¾è¿”å›žæ­£ç¡® Alarm Codeã€‚
      break;
    case 0x2E:
      DATA_Str_Serial = "Qt instruction failed, tag did not return data or returned data CRC calibration Check error.";
      //QT æŒ‡ä»¤å¤±è´¥ï¼Œæ ‡ç­¾æ²¡æœ‰è¿”å›žæ•°æ�®æˆ–è€…è¿”å›žæ•°æ�® CRC æ ¡éªŒé”™è¯¯ã€‚
      break;
    case 0xA3:
    case 0xB3:
    case 0xC3:
    case 0xD3:
    case 0xE3:
      DATA_Str_Serial = "Memory overrun.";
      //The specified label data store does not exist;Or the label does not support EPCs of a specified length, such as XPC.
      //æŒ‡å®šçš„æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºä¸�å­˜åœ¨ï¼›æˆ–è€…è¯¥æ ‡ç­¾ä¸�æ”¯æŒ�æŒ‡å®šé•¿åº¦çš„ EPCï¼Œæ¯”å¦‚ XPCã€‚
      break;
    case 0xA4:
    case 0xB4:
    case 0xC4:
    case 0xD4:
    case 0xE4:
      DATA_Str_Serial = "Memory locked.";
      //The specified label data store is locked and/or permanently locked, and the locked state is writable or unreadable
      //æŒ‡å®šçš„æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºè¢«é”�å®šå¹¶ä¸”/æˆ–è€…æ˜¯æ°¸ä¹…é”�å®šï¼Œè€Œä¸”é”�å®šçŠ¶æ€�ä¸ºä¸�å�¯å†™æˆ–ä¸�å�¯è¯»
      break;
    case 0xAB:
    case 0xBB:
    case 0xCB:
    case 0xDB:
    case 0xEB:
      DATA_Str_Serial = "Insufficient power.";
      //The tag did not receive enough energy to write
      //æ ‡ç­¾æ²¡æœ‰æ”¶åˆ°è¶³å¤Ÿçš„èƒ½é‡�æ�¥è¿›è¡Œå†™æ“�ä½œ
      break;
    case 0xAF:
    case 0xBF:
    case 0xCF:
    case 0xDF:
    case 0xEF:
      DATA_Str_Serial = "Non-specific error.";
      //The tag does not support error-code returns
      //æ ‡ç­¾ä¸�æ”¯æŒ� Error-code è¿”å›ž
      break;
    default: break;

  }

}

void UHF_RFID::Delay(unsigned int xms)
{

  Delay(xms);
}


UBYTE  UHF_RFID::DelayScanwarning()
{
  UDOUBLE i = 0;
  for ( i = 0; i < sizeof(DATA_I) - 6; i++)
  {
    if (DATA_I[i] == 0xBB && DATA_I[i + 2] == 0xFF )
    {
      Warningmessage(DATA_I[i + 5]);

      return DATA_I[i + 5];
    }

  }

  return 0;

}

void UHF_RFID::Copy_command_library( UBYTE com_nub)
{

  while (RFID_cmdnub[com_nub][DATA_Interim_b] != 0x7E) //First copy the command library å…ˆå¤�åˆ¶å‘½ä»¤åº“
  {
    DATA_Interim_order[DATA_Interim_b] = RFID_cmdnub[com_nub][DATA_Interim_b];
    DATA_Interim_b++;
  }
  DATA_Interim_order[DATA_Interim_b] = 0x7E;
}

void UHF_RFID::Check_bit_accumulation()
{
  DATA_Interim_order[DATA_Interim_b - 1] = 0x00;                      //Check bit accumulation æ ¡éªŒä½�ç´¯åŠ 

  for (UBYTE c = 1; c < DATA_Interim_b - 1; c++)
  {
    DATA_Interim_order[DATA_Interim_b - 1] = DATA_Interim_order[c] + DATA_Interim_order[DATA_Interim_b - 1] ;
  }
  if (DATA_Interim_order[DATA_Interim_b - 1] > 0xFF)
  {
    DATA_Interim_order[DATA_Interim_b - 1] =  DATA_Interim_order[DATA_Interim_b - 1] % 256;
  }
}

UWORD UHF_RFID::ToHex(UDOUBLE parameters, UBYTE MSB, UBYTE LSB)
{

  UBYTE i = LSB - MSB;
  UBYTE l = LSB;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  // DATA_Interim_order[3] = 0x12;             //Split the reshape to a command frame parameter æ‹†åˆ†æ•´å½¢è£…æ�¢ä¸ºå‘½ä»¤å¸§å�‚æ•°
  // DATA_Interim_order[4] = 0x34;

  if (i >= 4) {
    return 0;
  }
  if (parameters >= 0 && parameters < pow(256, i + 1))
  {
    while (parameters > 0 || l >= MSB)
    {
      if (l < MSB)
      {
        break;
      }
      DATA_Interim_order[l--] = parameters % 256;
      parameters = parameters / 256;
    }
  }
return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

UBYTE UHF_RFID::Verify_the_return(UBYTE a[] , UBYTE size_v)  //Verify return information éªŒè¯�è¿”å›žä¿¡æ�¯
{
  UBYTE c = 0;

  if ( DATA_I[0] == a[0] && DATA_I[1] == a[1] )
  {
    for (UBYTE i = 0; i < size_v; i++)
    {
      if (a[i] == DATA_I[i])
      {
        c++;
      }
    }
    if (c == size_v)
    {
      return 1;
    }

  }

  return 0;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Used to transfer string EPC to command frame

  ç”¨äºŽå°†å­—ç¬¦ä¸²EPCè½¬å‘½ä»¤å¸§
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*void UHF_RFID::EPC_string_to_command_frame(String str_epc, UBYTE MSB, UBYTE LSB)
{
  UBYTE b  = 0;
  b = LSB - MSB;

  String d1 = str_epc.substring(0, 8);
  String d2 = str_epc.substring(8, 16);
  String d3 = str_epc.substring(16, 24);

  if (b == 11)
  {
    ToHex(String_to_command_frame(d1), MSB, MSB + 3);
    ToHex(String_to_command_frame(d2), MSB + 4, MSB + 7);
    ToHex(String_to_command_frame(d3), MSB + 8, MSB + 11);
  }
}
*/
CardInformationInfo UHF_RFID::Access_Password_is_incorrect()
{

  CardInformationInfo Cardinformation;

  Cardinformation._UL = DATA_Str_M5led.substring( 12,  14);
  Cardinformation._PC = DATA_Str_M5led.substring( 14,  18);
  Cardinformation._EPC =DATA_Str_M5led.substring( 18,  42);
  Cardinformation._Parameter ="";
  Cardinformation._ErrorCode ="";
  Cardinformation._Error = "The Access Password is incorrect";
  Cardinformation._Data ="";
  Cardinformation._Successful ="";

  return Cardinformation;
}

CardInformationInfo UHF_RFID::EPC_Gen2_error_code()
{
  CardInformationInfo Cardinformation;

  Cardinformation._UL = DATA_Str_M5led.substring( 12,  14);
  Cardinformation._PC = DATA_Str_M5led.substring( 14,  18);
  Cardinformation._EPC =DATA_Str_M5led.substring( 18,  42);
  Cardinformation._Parameter ="";
  Cardinformation._ErrorCode =DATA_Str_M5led.substring( 10,  12);
  Cardinformation._Error = "EPC Gen2 error code";
  Cardinformation._Data ="";
  Cardinformation._Successful ="";
  
  return Cardinformation;

}

CardInformationInfo UHF_RFID::Operation_is_successful()
{
  CardInformationInfo Cardinformation;

  Cardinformation._UL = DATA_Str_M5led.substring( 10,  12);
  Cardinformation._PC = DATA_Str_M5led.substring( 12,  16);
  Cardinformation._EPC =DATA_Str_M5led.substring( 16,  40);
  Cardinformation._Parameter =DATA_Str_M5led.substring( 40,  42);
  Cardinformation._ErrorCode ="";
  Cardinformation._Error = "";
  Cardinformation._Data ="";
  Cardinformation._Successful ="";

  return Cardinformation;

}

CardInformationInfo UHF_RFID::UI_PC_EPC()
{
  CardInformationInfo Cardinformation;

  Cardinformation._UL = DATA_Str_M5led.substring( 10,  12);
  Cardinformation._PC = DATA_Str_M5led.substring( 12,  16);
  Cardinformation._EPC =DATA_Str_M5led.substring( 16,  40);
  Cardinformation._Parameter ="";
  Cardinformation._ErrorCode ="";
  Cardinformation._Error = "";
  Cardinformation._Data ="";
  Cardinformation._Successful ="";
  
  return Cardinformation;

}

