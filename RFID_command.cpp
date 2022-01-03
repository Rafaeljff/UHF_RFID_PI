#include "WString.h"
#include "RFID_command.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to ask about the hardware version
  ç”¨äºŽè¯¢é—®ç¡¬ä»¶ç‰ˆæœ¬
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Query_hardware_version()
{
  Sendcommand(0);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return DATA_Str_M5led;
  }
  else
  {
    Return_to_convert(0);

   return DATA_Str_M5led.substring(6, 21);
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to ask about software versions
  ç”¨äºŽè¯¢é—®è½¯ä»¶ç‰ˆæœ¬
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Query_software_version()
{
  Sendcommand(1);
  Delay(50);
  Readcallback();

  if (DelayScanwarning())
  {
    return DATA_Str_M5led;
  }
  else
  {
    Return_to_convert(0);

        return DATA_Str_M5led.substring(6, 21);
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to ask for RFID manufacturer information
  ç”¨äºŽè¯¢é—®RFIDåˆ¶é€ å•†ä¿¡æ�¯
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Inquire_manufacturer()
{
  Sendcommand(2);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return DATA_Str_M5led;
  }
  else
  {
    Return_to_convert(0);

    return DATA_Str_M5led.substring(6, 13);
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for a single polling instruction
  ç”¨äºŽå�•æ¬¡è½®è¯¢æŒ‡ä»¤
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardpropertiesInfo UHF_RFID::A_single_poll_of_instructions()
{
  UBYTE a[5] = {0xBB, 0x02, 0x22, 0x00, 0x11};
  CardpropertiesInfo card;

  Sendcommand(3);
  Delay(50);
  Readcallback();

  if (DelayScanwarning())
  {
    card._RSSI = "";
    card._PC = "";
    card._EPC = "";
    card._CRC = "";
    card._ERROR = DATA_Str_Serial;
    return card;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

  
      card._RSSI = DATA_Str_M5led.substring(10, 12);
      card._PC = DATA_Str_M5led.substring(12, 16);
      card._EPC = DATA_Str_M5led.substring(16, 40);
      card._CRC = DATA_Str_M5led.substring(40, 44);
      card._ERROR = "";

      return card;
    }
  }
   return card;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Used to poll instructions multiple times
  Cycle_nub: It is best to do between 0 and 100 reps, with a maximum of 65535 reps but too many reps will cause memory overflow

  ç”¨äºŽå¤šæ¬¡è½®è¯¢æŒ‡ä»¤
  cycle_nub: æœ€å¥½åœ¨0-100æ¬¡å†…ï¼Œæœ€å¤§æ˜¯65535æ¬¡ä½†æ˜¯æ¬¡æ•°å¤ªå¤šä¼šé€ æˆ�å†…å­˜æº¢å‡º
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ManyInfo UHF_RFID::Multiple_polling_instructions(UWORD cycle_nub)
{
  free(this->card);

  UWORD a = 0x00;
  UWORD b = 0x00;
  UBYTE d = 0;
  UWORD i_nub = 0;
  String data_mp = "";
  String data_tpy = "";

  Copy_command_library(4);
  ToHex(cycle_nub, 6, 7);
  Check_bit_accumulation();
  Send_the_modified_command();

  for (int i = 0; i != cycle_nub; i++)
  {
    Readcallback();

    i_nub = DATA_I_NUB + 1;

    Return_to_convert(1);

    while (i_nub > 0)
    {
      if (DATA_I[i_nub - 1] == 0x7E)
      {
        b = i_nub - 1;

      }
      if (DATA_I[i_nub - 1] == 0xBB && DATA_I[i_nub] == 0x02)
      {
        a = i_nub - 1;

      }
      if (b - a == 0x17   )
      {
        if (data_mp.indexOf(DATA_Str_M5led.substring(a * 2 + 16, a * 2 + 40)) == -1)
        {
          data_tpy = DATA_Str_M5led.substring(a * 2 + 10, a * 2 + 44);
          if (data_tpy.indexOf("bb01") == -1 )   //To prevent the mixing of EPC errors, should only be limited to the beginning of 0xbb command é˜²æ­¢æ··å…¥é”™è¯¯çš„EPCï¼Œåº”bbä»…é™�å‘½ä»¤å¼€å¤´
          {
            data_mp += DATA_Str_M5led.substring(a * 2 + 10, a * 2 + 44);
            d += 1;
          }
        }
        a = 0; b = 0;
      }
      i_nub--;
    }
    a = 0; b = 0;
  }

  this->card = (CardpropertiesInfo *)calloc(d, sizeof(CardpropertiesInfo));

  for (int i = 0; i != d; i++ )
  {
    this->card[i]._RSSI = data_mp.substring(i * 34 + 0, i * 34 + 2);
    this->card[i]._PC = data_mp.substring(i * 34 + 2, i * 34 + 6);
    this->card[i]._EPC = data_mp.substring(i * 34 + 6, i * 34 + 30);
    this->card[i]._CRC = data_mp.substring(i * 34 + 30, i * 34 + 34);
    this->card[i]._ERROR = "";
  }
  ManyInfo Cards = {d, this->card};

  return Cards;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to stop multiple polling instructions
  ç”¨äºŽå�œæ­¢å¤šæ¬¡è½®è¯¢æŒ‡ä»¤
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Stop_the_multiple_poll_directive()
{
  UBYTE a[8] = {0xBB, 0x01, 0x28, 0x00, 0x01, 0x00, 0x2A, 0x7E};

  Sendcommand(5);
  Delay(100);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {
      return "Stop OK";
    }
    else
    {
      return "Stop Unsuccess";
    }
  }
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Use to set the SELECT parameter instruction
  Str_epc: Example of the EPC String: "30751FEB705C5904E3D50D70"
  SelParam: SelParam contains 1 Byte, in which Target contains the highest 3 bits, Action contains the middle 3 bits,
  Membank occupies the last 2 bits.
  Membank means as follows:
  2 'B00: Label RFU data store
  2 'B01: Label EPC data store
  2 'B10: Tag TID data store
  2 'b11: label User data store
    See the EPC Gen2 protocol for the full meaning of Target and Action.
  PTR: 0x00000020(in bits, not words) starts from the EPC storage bit
  Mask: Length Masklen: 0x60(6 words, 96bits)
  Truncate: 0x00(0x00 is Disable truncation, 0x80 is Enable truncation)


  ç”¨äºŽè®¾ç½®Selectå�‚æ•°æŒ‡ä»¤

  str_epcï¼šEPCçš„å­—ç¬¦ä¸²ä¾‹ï¼š"30751FEB705C5904E3D50D70"
  SelParamï¼šSelParam å…± 1 ä¸ª Byteï¼Œå…¶ä¸­ Target å� æœ€é«˜ 3 ä¸ª bitsï¼ŒAction å� ä¸­é—´ 3 ä¸ª bitsï¼Œ
  MemBank å� æœ€å�Ž 2 ä¸ª bitsã€‚
            MemBank å�«ä¹‰å¦‚ä¸‹ï¼š
              2â€™b00:	æ ‡ç­¾ RFU æ•°æ�®å­˜å‚¨åŒº
              2â€™b01:	æ ‡ç­¾ EPC æ•°æ�®å­˜å‚¨åŒº
              2â€™b10:	æ ‡ç­¾ TID æ•°æ�®å­˜å‚¨åŒº
              2â€™b11:	æ ‡ç­¾ User æ•°æ�®å­˜å‚¨åŒº
                Target å’Œ Action è¯¦ç»†å�«ä¹‰è¯·å�‚è§� EPC Gen2 å��è®®ã€‚
  Ptr:	0x00000020(ä»¥ bit ä¸ºå�•ä½�ï¼Œé�ž word) ä»Ž EPC å­˜å‚¨ä½�å¼€å§‹
  Maskï¼š é•¿åº¦ MaskLen:	0x60(6 ä¸ª wordï¼Œ96bits)
  Truncate:	0x00(0x00 æ˜¯ Disable truncationï¼Œ0x80 æ˜¯ Enable truncation)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_the_select_parameter_directive(String str_epc, UBYTE SelParam, UDOUBLE Ptr, UBYTE MaskLen, UBYTE Truncate)
{
  UBYTE b[8] = {0xBB, 0x01, 0x0C, 0x00, 0x01, 0x00, 0x0E, 0x7E};

  Copy_command_library(6);
  ToHex(SelParam, 5, 5);//0x01
  ToHex(Ptr, 6, 9);//0x20
  ToHex(MaskLen, 10, 10);//0x30
  ToHex(Truncate, 11, 11);//0x00

  EPC_String_to_command_frame(str_epc, 12, 23);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(100);
  Readcallback();


  if (DelayScanwarning())
  {
    return "";
  }
  else
  {
    if (Verify_the_return(b, 8))
    {
      return "Set the select OK";
    }
    else
    {
      return "Set the select Unsuccess";
    }
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to get the SELECT parameter
  ç”¨äºŽèŽ·å�–Selectå�‚æ•°
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
SelectInfo UHF_RFID::Get_the_select_parameter()
{
  UBYTE a[4] = {0xBB, 0x01, 0x0B, 0x00};

  SelectInfo Select;

  Sendcommand(7);
  Delay(100);
  Readcallback();


  if (DelayScanwarning())
  {
    Select.Mask = "";
    Select.SelParam = "";
    Select.Ptr = "";
    Select.MaskLen = "";
    Select.Truncate = "";

    return Select;
  }
  else
  {
    if (Verify_the_return(a, 4))
    {
      Return_to_convert(1);

      Select.Mask = DATA_Str_M5led.substring( 24,  48);
      Select.SelParam = DATA_Str_M5led.substring( 10,  12);
      Select.Ptr = DATA_Str_M5led.substring( 12,  20);
      Select.MaskLen = DATA_Str_M5led.substring( 20,  22);
      Select.Truncate = DATA_Str_M5led.substring( 22,  24);

      return Select;
    }

  }
   return Select;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the SELECT mode
  Select Mode Mode
  0x00: The Select instruction is pre-sent to Select a particular label before any operation on the label.
  0x01: The SELECT instruction is not sent until the label is operated on.
  0x02: Send a SELECT instruction only before a label operation other than polling Inventory, as in
    Read, Write, Lock, and Kill are preceded by Select for a specific tag.

  ç”¨äºŽè®¾ç½®Selectæ¨¡å¼�

  Select æ¨¡å¼� Mode å�«ä¹‰ï¼š
    0x00:	åœ¨å¯¹æ ‡ç­¾çš„æ‰€æœ‰æ“�ä½œä¹‹å‰�éƒ½é¢„å…ˆå�‘é€� Select æŒ‡ä»¤é€‰å�–ç‰¹å®šçš„æ ‡ç­¾ã€‚
    0x01:	åœ¨å¯¹æ ‡ç­¾æ“�ä½œä¹‹å‰�ä¸�å�‘é€� Select æŒ‡ä»¤ã€‚
    0x02:	ä»…å¯¹é™¤è½®è¯¢ Inventory ä¹‹å¤–çš„æ ‡ç­¾æ“�ä½œä¹‹å‰�å�‘é€� Select æŒ‡ä»¤ï¼Œå¦‚åœ¨
          Readï¼ŒWriteï¼ŒLockï¼ŒKill ä¹‹å‰�å…ˆé€šè¿‡ Select é€‰å�–ç‰¹å®šçš„æ ‡ç­¾ã€‚
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_the_Select_mode(UBYTE Select_mode)
{
  UBYTE b[8] = {0xBB, 0x01, 0x12, 0x00, 0x01, 0x00, 0x14, 0x7E};

  Copy_command_library(8);
  ToHex(Select_mode, 5, 5);
  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();


  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(b, 8))
    {
      return "Set the select mode OK";
    }
    else
    {
      return "Set the select mode Unsuccess";
    }
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   For a single tag, read the tag data store

  Tag data store MemBank: 0x00(RFU area) 0x01(EPC area) 0x02(TID area) 0x03(USER area)
  Read label data area address offset SA: 0x0000
  Read label data area address length DL: 0x0002

  ç”¨äºŽå¯¹å�•ä¸ªæ ‡ç­¾ï¼Œè¯»å�–æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒº

  æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒº MemBank:  0x00(RFU åŒº) 0x01(EPC åŒº) 0x02(TID åŒº) 0x03(User åŒº)
  è¯»æ ‡ç­¾æ•°æ�®åŒºåœ°å�€å��ç§» SA:  0x0000
  è¯»æ ‡ç­¾æ•°æ�®åŒºåœ°å�€é•¿åº¦ DL:  0x0002

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::Read_the_label_data_store(UDOUBLE Access_Password, UBYTE MemBank, UWORD SA, UWORD DL)
{
  UBYTE b[6] = {0xBB, 0x01, 0x39, 0x00, 0x13, 0x0E};
  UBYTE e=0;

  CardInformationInfo Cardinformation;

  Copy_command_library(9);

  ToHex(Access_Password, 5, 8);
  ToHex(MemBank, 9, 9);
  ToHex(SA, 10, 11);
  ToHex(DL, 12, 13);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(50);
  Readcallback();


  if (e == DelayScanwarning())
  {

    if (e == 0x16)
    {
      Return_to_convert(1);

      Cardinformation = Access_Password_is_incorrect();
    }

    else if (e / 0x10 == 0xA )
    {
      Return_to_convert(1);

      Cardinformation = EPC_Gen2_error_code();
    }

    return Cardinformation;
  }
  else
  {
    if (Verify_the_return(b, 6))
    {
      Return_to_convert(1);

      Cardinformation = UI_PC_EPC();
      Cardinformation._Data = DATA_Str_M5led.substring( 40,  48);


      return Cardinformation;
    }


  }
   return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   For a single tag, write to the tag data store Memory Bank at the specified address and length.
  The length of the data to be written to the label data store, DT, should not exceed 32 words, or 64 bytes /512Bit bits.

  Label data store MEMBANK: 0x03
  Tag data area address offset SA: 0x0000
  Data length DL: 0x0002
  Write data DT: 0x12345678

  ç”¨äºŽå¯¹å�•ä¸ªæ ‡ç­¾ï¼Œå†™å…¥æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒº Memory Bank ä¸­æŒ‡å®šåœ°å�€å’Œé•¿åº¦çš„æ•°æ�®ã€‚
  å†™å…¥æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒºçš„æ•°æ�®é•¿åº¦ DT åº”ä¸�è¶…è¿‡ 32 ä¸ª wordï¼Œå�³ 64Byte å­—èŠ‚/512Bit ä½�.
                                  æ”¹å†™å¯†ç �Access_Password
  æ ‡ç­¾æ•°æ�®å­˜å‚¨åŒº MemBank:  0x03   (0x00)
  æ ‡ç­¾æ•°æ�®åŒºåœ°å�€å��ç§» SA:  0x0000  (0x02)
  æ•°æ�®é•¿åº¦ DL:  0x0002            (0x02)
  å†™å…¥æ•°æ�® DT:  0x12345678
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::The_label_store_writes_data(UDOUBLE Access_Password, UBYTE MemBank, UWORD SA, UWORD DL, UDOUBLE DT)
{

  CardInformationInfo Cardinformation;

  UBYTE b[6] = {0xBB, 0x01, 0x49, 0x00, 0x10, 0x0E};
  UBYTE e=0;

  Copy_command_library(10);

  ToHex(Access_Password, 5, 8);
  ToHex(MemBank, 9, 9);
  ToHex(SA, 10, 11);
  ToHex(DL, 12, 13);
  ToHex(DT, 14, 17);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(50);
  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);

      Cardinformation = Access_Password_is_incorrect();
    }

    else if (e / 0x10 == 0xB )
    {
      Return_to_convert(1);

      Cardinformation = EPC_Gen2_error_code();
    }


    return Cardinformation;
  }
  else
  {
    if (Verify_the_return(b, 6))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "Write to successful";

      return Cardinformation;
    }

  } return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to Lock the LOCK label data store
  For a single tag, Lock or Unlock the data store for that tag.
  ACTION_NUB has 0: kill 1: Access 2: EPC 3: TID 4: User
  Action has 2 bits, 00~11, corresponding to open, permanent open, lock, permanent lock

  ç”¨äºŽé”�å®šLockæ ‡ç­¾æ•°æ�®å­˜å‚¨åŒº

  å¯¹å�•ä¸ªæ ‡ç­¾ï¼Œé”�å®š Lock æˆ–è€…è§£é”� Unlock è¯¥æ ‡ç­¾çš„æ•°æ�®å­˜å‚¨åŒºã€‚
  Action_nub æœ‰ 0ï¼škill  1ï¼šAccess  2ï¼šEPC  3ï¼šTID  4ï¼šUser
  Action æœ‰ 2 bitsï¼Œ00~11ï¼Œä¾�æ¬¡å¯¹åº”ä¸º å¼€æ”¾ï¼Œæ°¸ä¹…å¼€æ”¾ï¼Œé”�å®šï¼Œæ°¸ä¹…é”�å®š
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::Lock_the_label_data_store(UDOUBLE Access_Password , UBYTE Action_nub, UBYTE Action )
{
  UDOUBLE Action_Data = 0;
  UBYTE a[6] = {0xBB, 0x01, 0x82, 0x00, 0x10, 0x0E};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  for (UBYTE i = 0; i < 10; i++)
  {
    if (i == Action_nub)
    {
      Action_Data = Action_Data * 0b100 + 0b10;
    }
    else if (i == Action_nub + 5)
    {
      Action_Data = Action_Data * 0b100 + Action;
    }
    else
    {
      Action_Data = Action_Data * 0b100;
    }
  }

  Copy_command_library(11);

  ToHex(Access_Password, 5, 8);
  ToHex(Action_Data, 9, 11);


  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(50);
  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }

    else if (e / 0x10 == 0xC )
    {
      Return_to_convert(1);
      Cardinformation = EPC_Gen2_error_code();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 6))
    {

      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      if (DATA_Str_M5led.substring( 40,  42) == "00")
      {
        if (Action == 0b01 || Action == 0b00)
        {
          Cardinformation._Successful = "unlock successful";
        }
        else if (Action == 0b10 || Action == 0b11)
        {
          Cardinformation._Successful = "Lock successful";
        }
      }
      return Cardinformation;
    }


  }
 return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to inactivate the Kill tag
  This directive should be preceded by setting the SELECT parameter to Select the specified label for the Kill inactivation operation.The inactivation of a single label.

  ç”¨äºŽç�­æ´»Killæ ‡ç­¾

  è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆè®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡å®šçš„æ ‡ç­¾è¿›è¡Œç�­æ´» Kill æ“�ä½œã€‚å¯¹å�•æ ‡ç­¾çš„ç�­æ´»æ“�ä½œã€‚
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::Inactivated_label(UDOUBLE Kill_Password)
{
  UBYTE a[6] = {0xBB, 0x01, 0x65, 0x00, 0x10, 0x0E};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(12);

  ToHex(Kill_Password, 5, 8);

  Check_bit_accumulation();
  Send_the_modified_command();

  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e / 0x10 == 0xD )
    {
      Return_to_convert(1);
      Cardinformation = EPC_Gen2_error_code();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 6))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      if (DATA_Str_M5led.substring( 40,  42) == "00")
      {
        Cardinformation._Successful = "Kill label successful";
      }
      return Cardinformation;
    }
}
  return Cardinformation ;
  }



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the communication baud rate
  Response frame defines that this instruction has no response frame.After the reader executes the command to set the communication baud rate, the reader communicates with the host computer using the new baud rate.
  The host machine needs to reconnect the reader with the new baud rate.
  Power parameter PoW: 0x00C0(in hexadecimal system of baud rate /100, such as 19200, that is 19200/100=192=0xC0)

  ç”¨äºŽè®¾ç½®é€šä¿¡æ³¢ç‰¹çŽ‡

  å“�åº”å¸§å®šä¹‰ è¯¥æŒ‡ä»¤æ²¡æœ‰å“�åº”å¸§ã€‚è¯»å†™å™¨æ‰§è¡Œå®Œè®¾ç½®é€šä¿¡æ³¢ç‰¹çŽ‡æŒ‡ä»¤å�Žï¼Œè¯»å†™å™¨å°±ä¼šç”¨æ–°çš„æ³¢ç‰¹çŽ‡ä¸Žä¸Šä½�æœºé€šä¿¡ï¼Œ
  ä¸Šä½�æœºéœ€ è¦�ç”¨æ–°çš„æ³¢ç‰¹çŽ‡é‡�æ–°è¿žæŽ¥è¯»å†™å™¨ã€‚

  åŠŸçŽ‡å�‚æ•° Pow:  0x00C0(æ³¢ç‰¹çŽ‡/100 çš„ 16 è¿›åˆ¶ï¼Œæ¯”å¦‚ 19200ï¼Œå°±æ˜¯ 19200/100=192=0xC0)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UBYTE UHF_RFID::Set_the_communication_baud_rate(UWORD Pow)
{

  Copy_command_library(13);

  ToHex(Pow, 5, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to get the Query parameters

  ç”¨äºŽèŽ·å�–Queryå�‚æ•°
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

QueryInfo UHF_RFID::Get_the_Query_parameter()
{
  String QueryStr = "";
  UBYTE a[5] = {0xBB, 0x01, 0x0D, 0x00, 0x02};
  UBYTE c = 0;
  UBYTE d = 0;
  //UBYTE e = 0;
  UBYTE i = 0;
  UBYTE x = 0;

  QueryInfo Query;

  Sendcommand(14);

  Readcallback();

  if (DelayScanwarning())
  {
    Query.QueryParameter = "";
    Query.DR = "";
    Query.M = "";
    Query.TRext = "";
    Query.Sel = "";
    Query.Session = "";
    Query.Target = "";
    Query.Q = "";

    return Query;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      for (int b = 5; b <= 6; b++)
      {
        i = 0;
        d = DATA_I[b];

        while ( i < 8)
        {
          c = d % 0b10;
          if (c != 0b0)
          {
            x = i;
          }
          d = d / 0b10;
          i++;

        }
        for (i = 1; i < 8 - x; i++)
        {
          QueryStr += "0";
        }
        QueryStr = QueryStr + String(DATA_I[b], 2);
      }
      Return_to_convert(1);

      Query.QueryParameter = DATA_Str_M5led.substring(10, 14);
      Query.DR = QueryStr.substring(0, 1);
      Query.M = QueryStr.substring(1, 3);
      Query.TRext = QueryStr.substring(3, 4);
      Query.Sel = QueryStr.substring(4, 6);
      Query.Session = QueryStr.substring(6, 8);
      Query.Target = QueryStr.substring(8, 9);
      Query.Q = QueryStr.substring(9, 13);

      return Query;
    }

  }
  return Query;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Use to set the Query parameter
  DR (1 -) : DR = 8 (1 'b0), DR = 64/3 b1 (1').Only DR=8 modes are supported
  M (2 -) : M = 1 (2 'b00), M = 2 (2' b01), M = 4 (2 'b10), M = 8 bl1 (2 ").Only M=1 modes are supported
  Trext (1 bit): No Pilot Tone (1 'B0), Use Pilot Tone (1' B1).Only Use Pilot Tone (1 'B1) mode is supported
  Sel (2 -) : ALL (2 'b01 b00/2 "), ~ SL (2' b10), SL (2 'b11)
  The Session (2 -) : S0 b00 (2 '), S1 (2 'b01), S2 (2' b10), S3 (2 'b11)
  The Target (1 -) : A (1 'b0), B (1' b1)
  Q: (bit 4) 4 'b0000-4' b1111

  ç”¨äºŽè®¾ç½®Queryå�‚æ•°

  DR(1 bit):	DR=8(1â€™b0), DR=64/3(1â€™b1). å�ªæ”¯æŒ� DR=8 çš„æ¨¡å¼�
  M(2 bit):	M=1(2â€™b00), M=2(2â€™b01), M=4(2â€™b10), M=8(2â€™b11). å�ªæ”¯æŒ� M=1 çš„æ¨¡å¼�
  TRext(1 bit):	No pilot tone(1â€™b0), Use pilot tone(1â€™b1). å�ªæ”¯æŒ� Use pilot tone(1â€™b1)æ¨¡å¼�
  Sel(2 bit):	ALL(2â€™b00/2â€™b01), ~SL(2â€™b10), SL(2â€™b11)
  Session(2 bit):	S0(2â€™b00), S1(2â€™b01), S2(2â€™b10), S3(2â€™b11)
  Target(1 bit):	A(1â€™b0), B(1â€™b1)
  Q(4 bit):	4â€™b0000-4â€™b1111
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

String UHF_RFID::set_the_Query_parameter(UBYTE Sel, UBYTE Session, UBYTE Target, UWORD Q)
{
  UDOUBLE para = 0b1;
  UBYTE a[8] = {0xBB, 0x01, 0x0E, 0x00, 0x01, 0x00, 0x10,};


  Copy_command_library(15);

  para =  para * 0b100 + Sel;
  para =  para * 0b100 + Session;
  para =  para * 0b10 + Target;
  para =  para * 0b10000 + Q;
  para =  para * 0b1000;

  ToHex(para, 5, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(50);

  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {
    if (Verify_the_return(a, 7))
    {
      return "Set the Query OK ";
    }
    else
    {
      return "Set the Query Unsuccess ";
    }


  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the reader work area
  China's 900 MHZ 0 x01
  China's 800 MHZ 0 x04
  The 0 x02
  European 0 x03
  South Korea 0 x06

  ç”¨äºŽè®¾ç½®è¯»å†™å™¨å·¥ä½œåœ°åŒº

  ä¸­å›½ 900MHz	0x01
  ä¸­å›½ 800MHz	0x04
  ç¾Žå›½	0x02
  æ¬§æ´²	0x03
  éŸ©å›½	0x06
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_up_work_area(UBYTE Region)
{
  UBYTE a[8] = {0xBB, 0x01, 0x07, 0x00, 0x01, 0x00, 0x09, 0x7E};

  Copy_command_library(16);

  ToHex(Region, 6, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {
      return "Set up work area OK ";
    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Used to read the reader work area
    China's 900 MHZ 0 x01
    China's 800 MHZ 0 x04
    The 0 x02
    European 0 x03
    South Korea 0 x06

  ç”¨äºŽè¯»å�–è¯»å†™å™¨å·¥ä½œåœ°åŒº
  ä¸­å›½ 900MHz  0x01
  ä¸­å›½ 800MHz 0x04
  ç¾Žå›½  0x02
  æ¬§æ´²  0x03
  éŸ©å›½  0x06
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ReadInfo UHF_RFID::Read_working_area()
{
  UBYTE a[5] = {0xBB, 0x01, 0x08, 0x00, 0x01};

  ReadInfo Read;

  Sendcommand(17);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    Read.Region = "";
    Read.Channel_Index = "";
    Read.Pow = "";
    Read.Mixer_G =  "";
    Read.IF_G =  "";
    Read.Thrd =  "";
    return Read;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);
      Read.Region = DATA_Str_M5led.substring(10, 12);
      Read.Channel_Index = "";
      Read.Pow = "";
      Read.Mixer_G =  "";
      Read.IF_G =  "";
      Read.Thrd =  "";

      return Read;
    }

  }
 return Read;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Used to set the reader working channel
  China 900MHz channel parameter calculation formula, FREQ_CH is the channel frequency:
  CH_Index = (Freq_CH to 920.125 M) / 0.25 M
  China 800MHz channel parameter calculation formula, Freq_CH is the channel frequency:
  CH_Index = (Freq_CH to 840.125 M) / 0.25 M
  American channel parameter calculation formula, Freq_CH is the channel frequency:
  CH_Index = (Freq_CH to 902.25 M) / 0.5 M
  European channel parameter calculation formula, Freq_CH is the channel frequency:
  CH_Index = (Freq_CH to 865.1 M) / 0.2 M
  Korean channel parameter calculation formula, Freq_CH is the channel frequency:
  CH_Index = (Freq_CH to 917.1 M) / 0.2 M
  If it is a Chinese 900MHz frequency band, set the reader working channel 920.375MHz, CH Index = 0x01

  ç”¨äºŽè®¾ç½®è¯»å†™å™¨å·¥ä½œä¿¡é�“
  ä¸­å›½ 900MHz ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  CH_Index = (Freq_CH-920.125M)/0.25M

  ä¸­å›½ 800MHz ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  CH_Index = (Freq_CH-840.125M)/0.25M

  ç¾Žå›½ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  CH_Index = (Freq_CH-902.25M)/0.5M

  æ¬§æ´²ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  CH_Index = (Freq_CH-865.1M)/0.2M

  éŸ©å›½ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  CH_Index = (Freq_CH-917.1M)/0.2M
  å¦‚æžœæ˜¯ä¸­å›½ 900MHz é¢‘æ®µï¼Œè®¾ç½®è¯»å†™å™¨å·¥ä½œä¿¡é�“ 920.375MHzï¼ŒCH Index = 0x01
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_up_working_channel(UBYTE CH_Index)
{
  UBYTE a[8] = {0xBB, 0x01, 0xAB, 0x00, 0x01, 0x00, 0xAD, 0x7E};

  Copy_command_library(18);

  ToHex(CH_Index, 6, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {

      return "Set up working channel OK ";
    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to read the reader working channel
  China 900MHz channel parameter calculation formula, FREQ_CH is the channel frequency:
  FREQ_CH = CH_INDEX * 0.25M + 920.125M
  China 800MHz channel parameter calculation formula, Freq_CH is the channel frequency:
  FREQ_CH = CH_INDEX * 0.25M + 840.125M
  American channel parameter calculation formula, Freq_CH is the channel frequency:
  FREQ_CH = CH_INDEX * 0.5m + 902.25m
  European channel parameter calculation formula, Freq_CH is the channel frequency:
  FREQ_CH = CH_INDEX * 0.2m + 865.1m
  Korean channel parameter calculation formula, Freq_CH is the channel frequency:
  FREQ_CH = CH_INDEX * 0.2m + 917.1m

  ç”¨äºŽè¯»å�–è¯»å†™å™¨å·¥ä½œä¿¡é�“

  ä¸­å›½ 900MHz ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  Freq_CH = CH_Index * 0.25M + 920.125M

  ä¸­å›½ 800MHz ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  Freq_CH = CH_Index * 0.25M + 840.125M

  ç¾Žå›½ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  Freq_CH = CH_Index * 0.5M + 902.25M

  æ¬§æ´²ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  Freq_CH = CH_Index * 0.2M + 865.1M

  éŸ©å›½ä¿¡é�“å�‚æ•°è®¡ç®—å…¬å¼�ï¼ŒFreq_CH ä¸ºä¿¡é�“é¢‘çŽ‡ï¼š
  Freq_CH = CH_Index * 0.2M + 917.1M

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ReadInfo UHF_RFID::Read_working_channel()
{
  UBYTE a[5] = {0xBB, 0x01, 0xAA, 0x00, 0x01};

  ReadInfo Read;

  Sendcommand(19);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    Read.Region = "";
    Read.Channel_Index = "";
    Read.Pow = "";
    Read.Mixer_G =  "";
    Read.IF_G =  "";
    Read.Thrd =  "";
    return Read;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);
      Read.Region = "";
      Read.Channel_Index = DATA_Str_M5led.substring(10, 12);
      Read.Pow = "";
      Read.Mixer_G =  "";
      Read.IF_G =  "";
      Read.Thrd =  "";
      return Read;
    }

  }
  return Read;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set up automatic frequency modulation
  Set the command frame definition to auto-hopping mode or cancel auto-hopping mode.In auto-hopping mode, if the user executes the insert working channel instruction,
  The reader randomly selects the channel hopping from the list of channels set by the user, otherwise it randomly selects the channel hopping from the internal preset list of channels.
  Parameter: 0xFF(0xFF means to set automatic frequency hopping, 0x00 means to cancel automatic frequency hopping)

  ç”¨äºŽè®¾ç½®è‡ªåŠ¨è°ƒé¢‘

  å‘½ä»¤å¸§å®šä¹‰ è®¾ç½®ä¸ºè‡ªåŠ¨è·³é¢‘æ¨¡å¼�æˆ–è€…å�–æ¶ˆè‡ªåŠ¨è·³é¢‘æ¨¡å¼�ã€‚åœ¨è‡ªåŠ¨è·³é¢‘æ¨¡å¼�ä¸‹ï¼Œå¦‚æžœç”¨æˆ·æ‰§è¡Œäº†æ�’å…¥å·¥ä½œä¿¡é�“æŒ‡ä»¤,
  åˆ™è¯»å†™ å™¨ä»Žç”¨æˆ·è®¾ç½®çš„ä¿¡é�“åˆ—è¡¨ä¸­éš�æœºé€‰æ‹©ä¿¡é�“è·³é¢‘ï¼Œå�¦åˆ™æŒ‰ç…§å†…éƒ¨é¢„è®¾çš„ä¿¡é�“åˆ—è¡¨éš�æœºé€‰æ‹©ä¿¡é�“è·³é¢‘ã€‚

  Parameter:	0xFF(0xFF ä¸ºè®¾ç½®è‡ªåŠ¨è·³é¢‘ï¼Œ0x00 ä¸ºå�–æ¶ˆè‡ªåŠ¨è·³é¢‘)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_up_automatic_frequency_modulation(UWORD Parameter)
{
  UBYTE a[8] = {0xBB, 0x01, 0xAD, 0x00, 0x01, 0x00, 0xAF, 0x7E};

  Copy_command_library(20);

  ToHex(Parameter, 6, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {
      if (Parameter == 0xff)
      {
        return "Set up aut FM Up ";
      }
      else if (Parameter == 0x00)
      {
        return "Set up aut FM Down";
      }

    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for plugging into a working channel
  Inserting a working channel allows the user to set the list of frequency hopping channels. After executing this command, the reader will randomly select from the list of channels set by the user
  Selective channel frequency hopping
  CH CNT: 0x05 (if 0, the list of frequency hopping channels is cleared, and the reader hops frequency randomly from all available channels)
  CH list: channel list (with CH Index): 0x01 0x02 0x03 0x04 0x05

  ç”¨äºŽæ�’å…¥å·¥ä½œä¿¡é�“

  æ�’å…¥å·¥ä½œä¿¡é�“å�¯ä»¥è®©ç”¨æˆ·è‡ªä¸»è®¾ç½®è·³é¢‘çš„ä¿¡é�“åˆ—è¡¨ï¼Œæ‰§è¡Œæ­¤å‘½ä»¤å�Žï¼Œè¯»å†™å™¨å°†ä»Žç”¨æˆ·è®¾ç½®çš„ä¿¡é�“åˆ—è¡¨ä¸­éš�æœºé€‰
  æ‹©ä¿¡é�“è·³é¢‘

  CH Cnt:	0x05 (å¦‚æžœä¸º 0ï¼Œåˆ™æ¸…é™¤è·³é¢‘ä¿¡é�“åˆ—è¡¨ï¼Œè¯»å†™å™¨ä»Žå…¨éƒ¨å�¯ç”¨ä¿¡é�“ä¸­éš�æœºè·³é¢‘)
  CH list : ä¿¡é�“åˆ—è¡¨(ç”¨ CH Index è¡¨ç¤º):	0x01 0x02 0x03 0x04 0x05
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Insert_working_channel(UBYTE CH_Cnt, UBYTE CH_list_1, UBYTE CH_list_2, UBYTE CH_list_3, UBYTE CH_list_4, UBYTE CH_list_5)
{
  UBYTE a[8] = {0xBB, 0x01, 0xA9, 0x00, 0x01, 0x00, 0xAB, 0x7E};

  Copy_command_library(21);

  ToHex(CH_Cnt, 5, 5);
  ToHex(CH_list_1, 6, 6);
  ToHex(CH_list_2, 7, 7);
  ToHex(CH_list_3, 8, 8);
  ToHex(CH_list_4, 9, 9);
  ToHex(CH_list_5, 10, 10);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {

      return "Insert working channel OK";
    }
    return "";

  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to obtain the current reader emission power

  ç”¨äºŽèŽ·å�–å½“å‰�è¯»å†™å™¨å�‘å°„åŠŸçŽ‡
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ReadInfo UHF_RFID::Read_transmitting_power()
{
  UBYTE a[5] = {0xBB, 0x01, 0xB7, 0x00, 0x02};

  ReadInfo Read;

  Sendcommand(22);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    Read.Region =  "";
    Read.Channel_Index = "";
    Read.Pow = "";
    Read.Mixer_G =  "";
    Read.IF_G =  "";
    Read.Thrd =  "";
    return Read;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);
      Read.Region =  "";
      Read.Channel_Index = "";
      Read.Pow = DATA_Str_M5led.substring(10, 14);
      Read.Mixer_G =  "";
      Read.IF_G =  "";
      Read.Thrd =  "";

      return Read;
    }

  }
   return Read;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the current reader emission power
  Power parameter PoW: 0x07D0(current power is decimal 2000, i.e. 20dBm)

  ç”¨äºŽè®¾ç½®å½“å‰�è¯»å†™å™¨å�‘å°„åŠŸçŽ‡

  åŠŸçŽ‡å�‚æ•° Pow:  0x07D0(å½“å‰�åŠŸçŽ‡ä¸ºå��è¿›åˆ¶ 2000ï¼Œå�³ 20dBm)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_transmission_Power(UWORD Pow)
{
  UBYTE a[8] = {0xBB, 0x01, 0xB6, 0x00, 0x01, 0x00, 0xB8, 0x7E};

  Copy_command_library(23);

  ToHex(Pow, 5, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {
    if (Verify_the_return(a, 8))
    {
      Return_to_convert(1);

      return "Set transmission Power OK";
    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set transmitting continuous carrier or to turn off continuous carrier
  Parameter: 0xFF(0xFF is to turn on CW, 0x00 is to turn off CW)

  ç”¨äºŽè®¾ç½®å�‘å°„è¿žç»­è½½æ³¢æˆ–è€…å…³é—­è¿žç»­è½½æ³¢

  æŒ‡ä»¤å�‚æ•° Parameter:	0xFF(0xFF ä¸ºæ‰“å¼€è¿žç»­æ³¢ï¼Œ0x00 ä¸ºå…³é—­è¿žç»­æ³¢)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Sets_to_transmit_a_continuous_carrier(UWORD Parameter)
{
  UBYTE a[8] = {0xBB, 0x01, 0xB0, 0x00, 0x01, 0x00, 0xB2, 0x7E};

  Copy_command_library(24);

  ToHex(Parameter, 5, 5);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {
    if (Verify_the_return(a, 8))
    {
      Return_to_convert(1);

      if (Parameter == 0xff)
      {
        return "Sets up TCC OK";
      }
      else if (Parameter == 0x00)
      {
        return "Turn off TCC OK";
      }

    }
    return "";
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to read receive demodulator parameters
  Gets the current reader receive demodulator parameters.The demodulator parameters are Mixer gain, IF AMP gain and signal demodulation threshold
  Mixer gain Mixer_G: 0x03(Mixer Mixer gain is 9dB)
  Gain IF_G: 0x06(IF AMP Gain: 36dB)
  Signal demodulation threshold THRD: 0x01B0(the smaller the signal demodulation threshold, the lower the return RSSI of the label that can be demodulated, but the more unstable,
  Not demodulated at all below a certain value;On the contrary, the larger the threshold is, the larger the RSSI of the label return signal can be demodulated, and the closer the distance is, the more stable it is.
  0x01B0 is the recommended minimum.)

    Mixer gain meter
    Type    Mixer_G (dB)
    0x00    0
    0x01    3
    0x02    6
    0x03    9
    0x04    12
    0x05    15
    0x06    16

    Gain meter of IF AMP
    Type    IF_G (dB)
    0x00    12
    0x01    18
    0x02    21
    0x03    24
    0x04    27
    0x05    30
    0x06    36
    0x07    40


  ç”¨äºŽè¯»å�–æŽ¥æ”¶è§£è°ƒå™¨å�‚æ•°

  èŽ·å�–å½“å‰�è¯»å†™å™¨æŽ¥æ”¶è§£è°ƒå™¨å�‚æ•°ã€‚è§£è°ƒå™¨å�‚æ•°æœ‰ Mixer å¢žç›Šï¼Œä¸­é¢‘æ”¾å¤§å™¨ IF AMP å¢žç›Šå’Œä¿¡å�·è§£è°ƒé˜ˆå€¼

    æ··é¢‘å™¨å¢žç›Š Mixer_G:           0x03(æ··é¢‘å™¨ Mixer å¢žç›Šä¸º 9dB)
  ä¸­é¢‘æ”¾å¤§å™¨å¢žç›Š IF_G:          0x06(ä¸­é¢‘æ”¾å¤§å™¨ IF AMP å¢žç›Šä¸º 36dB)
  ä¿¡å�·è§£è°ƒé˜ˆå€¼ Thrd:              0x01B0(ä¿¡å�·è§£è°ƒé˜ˆå€¼è¶Šå°�èƒ½è§£è°ƒçš„æ ‡ç­¾è¿”å›ž RSSI è¶Šä½Žï¼Œä½†è¶Šä¸�ç¨³å®šï¼Œ
  ä½ŽäºŽä¸€å®šå€¼å®Œå…¨ä¸�èƒ½è§£è°ƒï¼›ç›¸å��é˜ˆå€¼è¶Šå¤§èƒ½è§£è°ƒçš„æ ‡ç­¾è¿”å›žä¿¡å�· RSSI è¶Šå¤§ï¼Œè·�ç¦»è¶Šè¿‘ï¼Œè¶Šç¨³å®šã€‚
  0x01B0 æ˜¯æŽ¨è��çš„ æœ€å°�å€¼)

  æ··é¢‘å™¨ Mixer å¢žç›Šè¡¨       ä¸­é¢‘æ”¾å¤§å™¨ IF AMP å¢žç›Šè¡¨
    Type	Mixer_G(dB)             Type	IF_G(dB)
    0x00	0                        0x00	12
    0x01	3                        0x01	18
    0x02	6                        0x02	21
    0x03	9                        0x03	24
    0x04	12                       0x04	27
    0x05	15                       0x05	30
    0x06	16                       0x06	36
                                   0x07	40
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ReadInfo UHF_RFID::Read_receive_demodulator_parameters()
{
  UBYTE a[5] = {0xBB, 0x01, 0xF1, 0x00, 0x04};

  ReadInfo Read;

  Sendcommand(25);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    Read.Region =  "";
    Read.Channel_Index = "";
    Read.Pow = "";
    Read.Mixer_G = "";
    Read.IF_G =  "";
    Read.Thrd = "";
    return Read;
  }
  else
  {
    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Read.Region =  "";
      Read.Channel_Index = "";
      Read.Pow = "";
      Read.Mixer_G =  DATA_Str_M5led.substring(10, 12);
      Read.IF_G =  DATA_Str_M5led.substring(12, 14);
      Read.Thrd =  DATA_Str_M5led.substring(14, 18);

      return Read;
    }

  }
  return Read;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the receiving demodulator parameters
  Mixer gain Mixer_G: 0x03(Mixer Mixer gain is 9dB)
  Gain IF_G: 0x06(IF AMP Gain: 36dB)
  Signal demodulation threshold THRD: 0x01B0(The smaller the signal demodulation threshold, the lower the return RSSI of the label can be demodulated, but the more unstable it is
  , below a fixed value can not be demodulated at all;On the contrary, the larger the threshold is, the larger the RSSI of the label return signal can be demodulated, and the closer the distance is, the more stable it is.
  0x01B0 is the recommended minimum.)

    Mixer gain meter           Gain meter of IF AMP
   Type   Mixer_G (dB)           Type   IF_G (dB)
   0x00   0                      0x00   12
   0x01   3                      0x01   18
   0x02   6                      0x02   21
   0x03   9                      0x03   24
   0x04   12                     0x04   27
   0x05   15                     0x05   30
   0x06   16                     0x06   36

                                 0x07   40

  ç”¨äºŽè®¾ç½®æŽ¥æ”¶è§£è°ƒå™¨å�‚æ•°

  æ··é¢‘å™¨å¢žç›Š Mixer_G:           0x03(æ··é¢‘å™¨ Mixer å¢žç›Šä¸º 9dB)
  ä¸­é¢‘æ”¾å¤§å™¨å¢žç›Š IF_G:          0x06(ä¸­é¢‘æ”¾å¤§å™¨ IF AMP å¢žç›Šä¸º 36dB)
  ä¿¡å�·è§£è°ƒé˜ˆå€¼ Thrd:              0x01B0(ä¿¡å�·è§£è°ƒé˜ˆå€¼è¶Šå°�èƒ½è§£è°ƒçš„æ ‡ç­¾è¿”å›ž RSSI è¶Šä½Žï¼Œä½†è¶Šä¸�ç¨³å®š
  ï¼Œä½ŽäºŽä¸€ å®šå€¼å®Œå…¨ä¸�èƒ½è§£è°ƒï¼›ç›¸å��é˜ˆå€¼è¶Šå¤§èƒ½è§£è°ƒçš„æ ‡ç­¾è¿”å›žä¿¡å�· RSSI è¶Šå¤§ï¼Œè·�ç¦»è¶Šè¿‘ï¼Œè¶Šç¨³å®šã€‚
  0x01B0 æ˜¯æŽ¨è��çš„ æœ€å°�å€¼)

  æ··é¢‘å™¨ Mixer å¢žç›Šè¡¨       ä¸­é¢‘æ”¾å¤§å™¨ IF AMP å¢žç›Šè¡¨
    Type	Mixer_G(dB)             Type	IF_G(dB)
    0x00	0                        0x00	12
    0x01	3                        0x01	18
    0x02	6                        0x02	21
    0x03	9                        0x03	24
    0x04	12                       0x04	27
    0x05	15                       0x05	30
    0x06	16                       0x06	36
                                   0x07	40
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Sets_the_receiv_demodulator_parameters(UBYTE Mixer_G, UBYTE IF_G, UWORD Thrd)
{
  UBYTE a[8] = {0xBB, 0x01, 0xF0, 0x00, 0x01, 0x00, 0xF2, 0x7E};

  Copy_command_library(26);

  ToHex(Mixer_G, 5, 5);
  ToHex(IF_G, 6, 6);
  ToHex(Thrd, 7, 8);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {
    if (Verify_the_return(a, 8))
    {
      Return_to_convert(1);
      return "Sets up RDP OK";
    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to test RF input block signal
  Channel blocking signal JMR: 0 xf2f1f0efeceae8eaeceef0f1f5f5f5f6f5f5f5f5 (blocking signals of each channel
  JMRs are all represented by a signed Byte, where 0xF2 is -14dbm.

  ç”¨äºŽæµ‹è¯•å°„é¢‘è¾“å…¥ç«¯é˜»å¡žä¿¡å�·

  ä¿¡é�“é˜»å¡žä¿¡å�· JMR:  0xF2 F1 F0 EF EC EA E8 EA EC EE F0 F1 F5 F5 F5 F6 F5 F5 F5 F5(æ¯�ä¸ªä¿¡é�“çš„é˜»å¡žä¿¡å�·
  JMR éƒ½ç”¨ä¸€ä¸ªæœ‰ç¬¦å�·çš„ Byte è¡¨ç¤ºï¼Œå…¶ä¸­ 0xF2 ä¸º-14dBm)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TestInfo UHF_RFID::Test_the_RF_input_blocking_signal()
{
  UBYTE a[5] = {0xBB, 0x01, 0xF2, 0x00, 0x16};
  //UBYTE b = 0 ;
  //UBYTE c = 0;
  //UBYTE d = 0;
  //UBYTE e = 0;

  TestInfo Test;

  Sendcommand(27);

  Readcallback();

  if (DelayScanwarning())
  {
    Test.CH_L = "";
    Test.CH_H = "";
    for (size_t i = 0; i < 20; i++)
    {
      Test.Data[i] = "";
    }

    return Test;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Test.CH_L = DATA_Str_M5led.substring(10, 12);
      Test.CH_H = DATA_Str_M5led.substring(12, 14);
      for (size_t i = 0; i < 20; i++)
      {
        Test.Data[i] = DATA_Str_M5led.substring(14 + (i) * 2, 16 + (i) * 2);
      }

      return Test;
    }

  }return Test;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to test the RF input RSSI signal size
  Channel RSSI signal strength: 0 xbabababababababababababababababababababa (each channel
  RSSI is represented by a signed Byte, where 0xBA is -70dBm, which is the minimum RSSI that the reader can detect.

  ç”¨äºŽæµ‹è¯•å°„é¢‘è¾“å…¥ç«¯ RSSI ä¿¡å�·å¤§å°�

  ä¿¡é�“ä¿¡å�·å¼ºåº¦ RSSI:  0xBA BA BA BA BA BA BA BA BA BA BA BA BA BA BA BA BA BA BA BA(æ¯�ä¸ªä¿¡é�“çš„
  RSSI ç”¨ä¸€ä¸ªæœ‰ç¬¦å�·çš„ Byte è¡¨ç¤ºï¼Œå…¶ä¸­ 0xBA ä¸º-70dBmï¼Œä¸ºè¯»å†™å™¨èƒ½æ£€æµ‹çš„ RSSI çš„æœ€å°�å€¼)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TestInfo UHF_RFID::Test_the_RSSI_input_signal()
{
  UBYTE a[5] = {0xBB, 0x01, 0xF3, 0x00, 0x16};
  //UBYTE b = 0;
  //UBYTE c = 0;
  //UBYTE d = 0;
  //UBYTE e = 0;

  TestInfo Test;

  Sendcommand(28);

  Readcallback();

  if (DelayScanwarning())
  {
    Test.CH_L = "";
    Test.CH_H = "";
    for (size_t i = 0; i < 20; i++)
    {
      Test.Data[i] = "";
    }

    return Test;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Test.CH_L = DATA_Str_M5led.substring(10, 12);
      Test.CH_H = DATA_Str_M5led.substring(12, 14);
      for (size_t i = 0; i < 20; i++)
      {
        Test.Data[i] = DATA_Str_M5led.substring(14 + (i) * 2, 16 + (i) * 2);
      }

      return Test;
    }
  } return Test;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for module sleep
  The module sleep instruction can keep the module in low power sleep mode.After the module sleeps, it can wake up the module by sending any byte through the serial port.
  But the byte will be discarded, and the first instruction received after the module sleeps will have no response because the first character of the first instruction will be discarded.
  This command will reset the M100/QM100 chip when the power is off. After waking up, the module will immediately download the firmware to the M100/QM100 chip and reset it
  Put some parameters into the module (these parameters include pre-sleep configuration power, frequency, frequency hopping mode, sleep time, receive demodulator parameters, not included
  Select mode, Select parameters, etc.), so some parameters may need to be reset.

  ç”¨äºŽæ¨¡å�—ä¼‘çœ 

  æ¨¡å�—ä¼‘çœ æŒ‡ä»¤å�¯ä»¥è®©æ¨¡å�—ä¿�æŒ�ä½ŽåŠŸè€—çš„ä¼‘çœ æ¨¡å¼�ã€‚æ¨¡å�—ä¼‘çœ å�Žï¼Œé€šè¿‡ä¸²å�£å�‘é€�ä»»æ„�çš„å­—èŠ‚å�³å�¯å”¤é†’æ¨¡å�—ï¼Œ
  ä½†è¯¥ å­—èŠ‚ä¼šè¢«æŠ›å¼ƒæŽ‰ï¼Œæ¨¡å�—ä¼‘çœ å�ŽæŽ¥æ”¶åˆ°çš„ç¬¬ä¸€æ�¡æŒ‡ä»¤ä¼šæ²¡æœ‰å“�åº”ï¼Œå› ä¸ºç¬¬ä¸€æ�¡æŒ‡ä»¤çš„ç¬¬ä¸€ä¸ªå­—ç¬¦ä¼šè¢«æŠ›å¼ƒæŽ‰ã€‚
  è¯¥ æŒ‡ä»¤ä¼šè®© M100/QM100 èŠ¯ç‰‡æŽ‰ç”µé‡�ç½®ï¼Œæ¨¡å�—å”¤é†’å�Žä¼šç«‹åˆ»é‡�æ–°ä¸‹è½½å›ºä»¶åˆ° M100/QM100 èŠ¯ç‰‡ä¸­ï¼Œå¹¶é‡�æ–°è®¾
  ç½®ä¸€äº›å�‚æ•°åˆ°æ¨¡å�—ä¸­ï¼ˆè¿™äº›å�‚æ•°åŒ…æ‹¬ä¼‘çœ å‰�é…�ç½®çš„åŠŸçŽ‡ï¼Œé¢‘çŽ‡ï¼Œè·³é¢‘æ¨¡å¼�ï¼Œä¼‘çœ æ—¶é—´ï¼ŒæŽ¥æ”¶è§£è°ƒå™¨å�‚æ•°ï¼Œä¸�åŒ…
  æ‹¬ Select æ¨¡å¼�ï¼ŒSelect å�‚æ•°ç­‰ï¼‰ï¼Œå› æ­¤æœ‰äº›å�‚æ•°å�¯èƒ½éœ€è¦�é‡�æ–°è®¾ç½®ã€‚
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_module_hibernation()
{
  UBYTE a[8] = {0xBB, 0x01, 0x17, 0x00, 0x01, 0x00, 0x19, 0x7E};

  Sendcommand(30);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {
      return "The module of dormancy OK";
    }
    return "";
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for the idle sleep time of the module
  This command can set how long the module will automatically enter the sleep state after no operation.After module sleep, send arbitrary via serial port
  The character can wake up the module.The first instruction received by the module after hibernation will not respond because the first character of the first instruction will be discarded.
  This command will reset the M100/QM100 chip. After waking up, the module will immediately download the firmware to the M100/QM100 chip and reset it
  Some parameters to the module (these parameters include pre-sleep configuration of power, frequency, frequency hopping mode, sleep time, receiving demodulator parameters, not included
  Select mode, Select parameters, etc.), so some parameters may need to be reset.
  Parameter: 0x02(Sleep after 2 minutes without operation, range 1 to 30 minutes, 0x00 represents no automatic sleep)

  ç”¨äºŽæ¨¡å�—ç©ºé—²ä¼‘çœ æ—¶é—´

  ä»¤å¸§å®šä¹‰ è¯¥æŒ‡ä»¤å�¯ä»¥è®¾ç½®æ¨¡å�—ç»�è¿‡å¤šé•¿æ—¶é—´æ²¡æœ‰æ“�ä½œå�Žè‡ªåŠ¨è¿›å…¥ä¼‘çœ çŠ¶æ€�ã€‚æ¨¡å�—ä¼‘çœ å�Žï¼Œé€šè¿‡ä¸²å�£å�‘é€�ä»»æ„�
  çš„å­—ç¬¦å�³å�¯ å”¤é†’æ¨¡å�—ã€‚æ¨¡å�—ä¼‘çœ å�ŽæŽ¥æ”¶åˆ°çš„ç¬¬ä¸€æ�¡æŒ‡ä»¤ä¼šæ²¡æœ‰å“�åº”ï¼Œå› ä¸ºç¬¬ä¸€æ�¡æŒ‡ä»¤çš„ç¬¬ä¸€ä¸ªå­—ç¬¦ä¼šè¢«æŠ›å¼ƒæŽ‰ã€‚
  è¯¥æŒ‡ä»¤ä¼š è®© M100/QM100 èŠ¯ç‰‡é‡�ç½®ï¼Œæ¨¡å�—å”¤é†’å�Žä¼šç«‹åˆ»é‡�æ–°ä¸‹è½½å›ºä»¶åˆ° M100/QM100 èŠ¯ç‰‡ä¸­ï¼Œå¹¶é‡�æ–°è®¾ç½®
  ä¸€äº›å�‚æ•°åˆ°æ¨¡å�—ä¸­ï¼ˆè¿™äº›å�‚æ•°åŒ…æ‹¬ä¼‘çœ å‰�é…�ç½®çš„åŠŸçŽ‡ï¼Œé¢‘çŽ‡ï¼Œè·³é¢‘æ¨¡å¼�ï¼Œä¼‘çœ æ—¶é—´ï¼ŒæŽ¥æ”¶è§£è°ƒå™¨å�‚æ•°ï¼Œä¸�åŒ…æ‹¬
  Select æ¨¡å¼�ï¼ŒSelect å�‚æ•°ç­‰ï¼‰ï¼Œå› æ­¤æœ‰äº›å�‚æ•°å�¯èƒ½éœ€è¦�é‡�æ–°è®¾ç½®ã€‚

  æŒ‡ä»¤å�‚æ•° Parameter:	0x02(2 åˆ†é’Ÿæ— æ“�ä½œä¹‹å�Žä¼‘çœ ï¼ŒèŒƒå›´ 1~30 åˆ†é’Ÿï¼Œ0x00 ä»£è¡¨ä¸�è‡ªåŠ¨ä¼‘çœ )
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_Sleep_Time(UWORD Parameter)
{
  UBYTE a[5] = {0xBB, 0x01, 0x1D, 0x00, 0x01};

  Copy_command_library(31);

  ToHex(Parameter, 6, 6);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);
      return "sleep time :" + DATA_Str_M5led.substring(10, 12) + "Set the sleep time OK" ;
    }
    return "";
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used to set the module ILDE mode
  This instruction can make the module enter the IDLE working mode. In the IDLE mode, except for the digital part and the communication interface, all the analog and RF parts are left
  The power supply is turned off to reduce power consumption when not working.After the module enters IDLE mode, it can still communicate with the module normally, and the parameters have been set
  The module is still saved and can respond to the instructions of the host computer.After entering IDLE mode, the label data is read and written for the first time
  Such instructions that need to interact with the label will restore the module to its normal state, but the first inventory may be due to the unstable state of the power supply in the RF part
  As a result, the success rate decreases, and the subsequent inventory and other operations can be restored to normal.
  Enter: 0x01(Enter IDLE mode, 0x00: exit IDLE mode)
  Reserved, fixed at 0x01
  Idle Time: 0x03(3 minutes without operation automatically into IDLE mode, range 0-30 minutes,
  0 x00 said don't automatically into IDLE mode)

  ç”¨äºŽè®¾ç½®æ¨¡å�—ILDE æ¨¡å¼�

  è¯¥æŒ‡ä»¤å�¯ä»¥è®©æ¨¡å�—è¿›å…¥ IDLE å·¥ä½œæ¨¡å¼�ï¼ŒIDLE æ¨¡å¼�ä¸‹é™¤äº†æ•°å­—éƒ¨åˆ†å’Œé€šä¿¡æŽ¥å�£ï¼Œå…¶ä½™æ‰€æœ‰æ¨¡æ‹Ÿå’Œå°„é¢‘éƒ¨åˆ†
  ç”µæº�å�‡è¢«å…³é—­ï¼Œä»¥å‡�å°‘ä¸�å·¥ä½œæƒ…å†µä¸‹çš„åŠŸè€—ã€‚æ¨¡å�—è¿›å…¥ IDLE æ¨¡å¼�å�Žï¼Œä¸Žæ¨¡å�—ä»�å�¯æ­£å¸¸é€šä¿¡ï¼Œå·²è®¾ç½®çš„å�‚æ•°
  ä»�ç„¶ è¢«ä¿�å­˜ï¼Œæ¨¡å�—å�¯ä»¥æ­£å¸¸å“�åº”ä¸Šä½�æœºçš„æŒ‡ä»¤ã€‚è¿›å…¥ IDLE æ¨¡å¼�å�Žï¼Œç¬¬ä¸€æ¬¡ç›˜ç‚¹ï¼ˆæˆ–è¯»å�–ï¼Œå†™å…¥æ ‡ç­¾æ•°æ�®
  æ­¤ç±»éœ€è¦�ä¸Žæ ‡ç­¾äº¤äº’çš„æŒ‡ä»¤ï¼‰ä¼šè®©æ¨¡å�—æ�¢å¤�åˆ°æ­£å¸¸çŠ¶æ€�ï¼Œä½†ç¬¬ä¸€æ¬¡ç›˜ç‚¹å�¯èƒ½ç”±äºŽå°„é¢‘éƒ¨åˆ†ç”µæº�çŠ¶æ€�ä¸�ç¨³å®šå¯¼
  è‡´æˆ�åŠŸçŽ‡ä¸‹é™�ï¼Œå�Žç»­çš„ç›˜ç‚¹å’Œå…¶ä»–æ“�ä½œå�³å�¯æ�¢å¤�æ­£å¸¸ã€‚

  æ˜¯å�¦è¿›å…¥ IDLE æ¨¡å¼� Enter:	0x01(è¿›å…¥ IDLE æ¨¡å¼�ï¼Œ0x00:é€€å‡º IDLE æ¨¡å¼�)
  æŒ‡ä»¤å�‚æ•° Reserved:	0x01(ä¿�ç•™ï¼Œå›ºå®šä¸º 0x01)
  IDLE æ¨¡å¼�ç©ºé—²æ—¶é—´ IDLE Time:	0x03(3 åˆ†é’Ÿæ— æ“�ä½œè‡ªåŠ¨è¿›å…¥ IDLE æ¨¡å¼�ï¼ŒèŒƒå›´ 0-30 åˆ†é’Ÿï¼Œ
                                0x00 è¡¨ç¤ºä¸�è‡ªåŠ¨ è¿›å…¥ IDLE æ¨¡å¼�)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::Set_the_ILDE_mode(UBYTE Enter, UBYTE IDLE_Time)
{
  UBYTE a[8] = {0xBB, 0x01, 0x04, 0x00, 0x01, 0x00, 0x06, 0x7E};

  Copy_command_library(32);

  ToHex(Enter, 5, 5);
  ToHex(IDLE_Time, 7, 7);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 8))
    {
      Return_to_convert(1);
      return "ILDE time :" + String(DATA_Interim_order[7]) + "Set ILDE OK";
    }
    return "";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   For NXP ReadProtect/Reset ReadProtect
  The NXP G2X label supports ReadProtect/Reset ReadProtect commands.When the tag executes the ReadProtect instruction successfully,
  The ProtectEPC and ProtectTID bits of the tag will be set to '1' and the tag will enter the data protection state.If you let the tag
  To return from the data protection state to the normal state, you need to execute the Reset ReadProtect command.
  This directive should be preceded by setting the SELECT parameter to Select the specified TAB to operate on.
  ReadProtect: 0x00(0x00 represents the execution of ReadProtect,
  0x01 means to execute Reset ReadProtect)

  ç”¨äºŽNXP ReadProtect/Reset ReadProtect

  NXP G2X æ ‡ç­¾æ”¯æŒ� ReadProtect/Reset ReadProtect æŒ‡ä»¤ã€‚å½“æ ‡ç­¾æ‰§è¡Œ ReadProtect æŒ‡ä»¤æˆ�åŠŸï¼Œ
  æ ‡ç­¾çš„ ProtectEPCå’ŒProtectTID ä½�å°†ä¼šè¢«è®¾ç½®ä¸ºâ€™1â€™ï¼Œæ ‡ç­¾ä¼šè¿›å…¥åˆ°æ•°æ�®ä¿�æŠ¤çš„çŠ¶æ€�ã€‚å¦‚æžœè®©æ ‡ç­¾
  ä»Žæ•°æ�®ä¿�æŠ¤çŠ¶æ€� å›žåˆ°æ­£å¸¸çŠ¶æ€�ï¼Œéœ€è¦�æ‰§è¡ŒReset ReadProtect æŒ‡ä»¤ã€‚
  è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆè®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡å®šçš„æ ‡ ç­¾è¿›è¡Œæ“�ä½œã€‚

  ReadProtect/Reset ReadProtect:	0x00(0x00 ä»£è¡¨æ‰§è¡Œ ReadProtectï¼Œ
                                  0x01 ä»£è¡¨æ‰§è¡Œ Reset ReadProtect)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::NXP_ReadProtect_ResetReadProtect(UDOUBLE Access_Password, UBYTE ReadProtect)
{
  UBYTE a[5] = {0xBB, 0x01, 0xE1, 0x00, 0x10};
  UBYTE b[5] = {0xBB, 0x01, 0xE2, 0x00, 0x10};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(33);

  ToHex(Access_Password, 5, 8);
  ToHex(ReadProtect, 9, 9);

  Check_bit_accumulation();
  Send_the_modified_command();
  Delay(50);
  Readcallback();

  if (e ==DelayScanwarning())
  {

    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "NXP ReadProtect OK";

      return Cardinformation;
    }
    else if (Verify_the_return(b, 5))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "NXP Reset ReadProtect OK";

      return Cardinformation;
    }

  }return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for NXP Change EAS instructions
  The NXP G2X label supports the CHANGE EAS directive.When the tag executes the CHANGE EAS directive successfully, the tag's PSF bit will correspond
  It becomes a '1' or a '0'.When the tag's PSF position is' 1 ', the tag will respond to the EAS_ALARM instruction, otherwise the tag will not respond
  EAS_Alarm instructions.This directive should be preceded by setting the SELECT parameter to Select the specified label to operate on.
  PSF: 0x01(0x01 represents setting PSF bit to '1', 0x00 represents setting PSF bit to '0')

  ç”¨äºŽNXP Change EAS æŒ‡ä»¤

  NXP G2X æ ‡ç­¾æ”¯æŒ� Change EAS æŒ‡ä»¤ã€‚å½“æ ‡ç­¾æ‰§è¡Œ Change EAS æŒ‡ä»¤æˆ�åŠŸï¼Œæ ‡ç­¾çš„ PSF ä½�å°†ä¼šç›¸åº”çš„
  å�˜æˆ�â€™1â€™ æˆ–è€…â€™0â€™ã€‚å½“æ ‡ç­¾çš„ PSF ä½�ç½®ä¸ºâ€™1â€™çš„æ—¶å€™ï¼Œæ ‡ç­¾å°†å“�åº” EAS_Alarm æŒ‡ä»¤ï¼Œå�¦åˆ™æ ‡ç­¾ä¸�å“�åº”
  EAS_Alarm æŒ‡ä»¤ã€‚ è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆè®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡å®šçš„æ ‡ç­¾è¿›è¡Œæ“�ä½œã€‚

  PSF:	0x01(0x01 ä»£è¡¨è®¾ç½® PSF ä½�ä¸ºâ€™1â€™ï¼Œ0x00 ä»£è¡¨è®¾ç½® PSF ä½�ä¸ºâ€™0â€™)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::NXP_Change_EAS(UDOUBLE Access_Password, UBYTE PSF)
{
  UBYTE a[5] = {0xBB, 0x01, 0xE3, 0x00, 0x10};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(34);

  ToHex(Access_Password, 5, 8);
  ToHex(PSF, 9, 9);

  Check_bit_accumulation();
  Send_the_modified_command();

  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "NXP Change EAS OK";

      return Cardinformation;
    }

  }
return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for the NXP EAS_ALARM instruction
  The NXP G2X tag supports the EAS_ALARM directive.When the tag receives the EAS_ALARM instruction, the tag returns immediately
  64 bits EAS - Alarm code.Note that the tag responds to EAS_Alarm only if the tag's PSF position is' 1 '.
  Otherwise the label will not respond to the EAS_ALARM instruction.This instruction is suitable for electronic goods anti - theft (theft) system.

  ç”¨äºŽNXP EAS_Alarm æŒ‡ä»¤

  NXP G2X æ ‡ç­¾æ”¯æŒ� EAS_Alarm æŒ‡ä»¤ã€‚å½“æ ‡ç­¾æŽ¥æ”¶åˆ° EAS_Alarm æŒ‡ä»¤å�Žï¼Œæ ‡ç­¾ä¼šç«‹åˆ»è¿”å›ž
  64bits EAS-Alarm codeã€‚æ³¨æ„�å�ªæœ‰å½“æ ‡ç­¾çš„ PSF ä½�ç½®ä¸ºâ€™1â€™çš„æ—¶å€™ï¼Œæ ‡ç­¾æ‰�å“�åº” EAS_Alarm æŒ‡ä»¤ï¼Œ
  å�¦åˆ™æ ‡ç­¾ä¸�å“�åº” EAS_Alarm æŒ‡ä»¤ã€‚è¯¥æŒ‡ä»¤é€‚å�ˆäºŽç”µå­�å•†å“�é˜²çªƒï¼ˆç›—ï¼‰ç³»ç»Ÿã€‚

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String UHF_RFID::NXP_EAS_Alarm()
{
  UBYTE a[8] = {0xBB, 0x01, 0xE4, 0x00, 0x08};

  Sendcommand(35);
  Delay(20);
  Readcallback();

  if (DelayScanwarning())
  {
    return "";
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      return "EAS-Alarm code :" + DATA_Str_M5led.substring(10, 26);
    }
    return "";
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for the NXP ChangeConfig directive
  Certain families of the NXG2X tag (such as G2IM and G2IM +) support the ChangeConfig directive, which can be read or modified
  NXP G2X label 16bits config-word.The config-word of the NXP G2X label is located in the label store BANK 01 (i.e
  EPC area) address 20h (word address), can be Read through the ordinary Read instruction.When the label is in the Secured state
  (security state), you can overwrite the config-word of the label, it should be noted that to rewrite the config-word is to flip the config-word
  , that is, the corresponding bit for writing '1' is flipped (' 1 'becomes' 0', '0' becomes' 1 '), and the corresponding bit for writing '0' remains unchanged.
  This instruction should be preceded by setting the SELECT parameter to Select the specified label to operate on.
  Config-word: 0x0000(all 0 label returns unchanged config-word, equivalent to read)

  ç”¨äºŽNXP ChangeConfig æŒ‡ä»¤

  NXP G2X æ ‡ç­¾æŸ�äº›ç³»åˆ—ï¼ˆå¦‚ G2iM å’Œ G2iM+ï¼‰æ”¯æŒ� ChangeConfig æŒ‡ä»¤ï¼Œå�¯ä»¥é€šè¿‡è¯¥æŒ‡ä»¤è¯»å�–æˆ–ä¿®æ”¹
  NXP G2X æ ‡ç­¾çš„ 16bits Config-Wordã€‚NXP G2X æ ‡ç­¾çš„ Config-Word ä½�äºŽæ ‡ç­¾å­˜å‚¨åŒº Bank 01ï¼ˆå�³
  EPC åŒºï¼‰åœ°å�€ 20h å¤„ï¼ˆword addressï¼‰ï¼Œå�¯ä»¥é€šè¿‡æ™®é€šçš„ Read æŒ‡ä»¤è¯»å�–ã€‚å½“æ ‡ç­¾å¤„äºŽ Secured çŠ¶æ€�
  ï¼ˆå®‰å…¨çŠ¶æ€�ï¼‰æ—¶ï¼Œå�¯ä»¥æ”¹å†™ æ ‡ç­¾çš„ Config-Wordï¼Œéœ€è¦�æ³¨æ„�çš„æ˜¯æ”¹å†™ Config-Word æ˜¯ç¿»è½¬ Config-Word
  çš„ç›¸åº”æ•°æ�®ä½�ï¼Œå�³å†™å…¥â€™1â€™çš„å¯¹åº” ä½�ç¿»è½¬ï¼ˆâ€˜1â€™å�˜æˆ�â€˜0â€™ï¼Œâ€˜0â€™å�˜æˆ�â€˜1â€™ï¼‰ï¼Œå†™å…¥â€˜0â€™çš„å¯¹åº”ä½�ä¿�æŒ�ä¸�å�˜ã€‚
  è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆè®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡ å®šçš„æ ‡ç­¾è¿›è¡Œæ“�ä½œã€‚

  Config-Word:	0x0000(å…¨ 0 æ—¶æ ‡ç­¾è¿”å›žæœªæ›´æ”¹çš„ Config-Wordï¼Œç›¸å½“äºŽè¯»å�–)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::NXP_Change_Config(UDOUBLE Access_Password, UWORD Config_Word)
{
  UBYTE a[5] = {0xBB, 0x01, 0xE0, 0x00, 0x11};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(36);

  ToHex(Access_Password, 5, 8);
  ToHex(Config_Word, 9, 10);

  Check_bit_accumulation();
  Send_the_modified_command();

  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Cardinformation = UI_PC_EPC();
      Cardinformation._Successful = "Config Word :" + DATA_Str_M5led.substring( 40,  44) + "NXP Change Config OK";

      return Cardinformation;
    }

  }
return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for the IMPINJ Monza QT directive
  Impinj Monza 4 Qt tags support Qt instructions that can modify the tag's Qt Control Word, where the QT_SR bit is set
  You can shorten the actions of the label in the Open and Secured states or when it is about to enter the Open and Secured states
  The QT_MEM bit can be used to switch between a Public Memory Map and a Private Memory Map
  (private storage).
  This directive should be preceded by setting the SELECT parameter to Select the specified label to operate on.
  Read/Write: 0x01(0x00: Read, 0x01: Write)
  Persistence: 0x01(0x00: Write to label volatile storage, 0x01: Write to non-volatile storage)
  Payload: 0x4000(Qt Control, maximum two bits QT_SR and QT_MEM)

  ç”¨äºŽImpinj Monza QT æŒ‡ä»¤

  Impinj Monza 4 QT æ ‡ç­¾æ”¯æŒ� QT æŒ‡ä»¤ï¼Œè¯¥æŒ‡ä»¤å�¯ä»¥ä¿®æ”¹æ ‡ç­¾çš„ QT Control wordï¼Œå…¶ä¸­è®¾ç½® QT_SR ä½�
  å�¯ä»¥ç¼© çŸ­æ ‡ç­¾åœ¨ Openï¼ˆå¼€æ”¾ï¼‰å’Œ Securedï¼ˆå®‰å…¨ï¼‰çŠ¶æ€�æˆ–è€…å�³å°†è¿›å…¥åˆ° Open å’Œ Secured çŠ¶æ€�æ—¶çš„æ“�ä½œ
  è·�ç¦»ï¼Œä¿®æ”¹ QT_MEM ä½�å�¯ä»¥åˆ‡æ�¢æ ‡ç­¾ä½¿ç”¨ Public Memory Mapï¼ˆå…¬å…±å­˜å‚¨åŒºï¼‰è¿˜æ˜¯ Private Memory Map
  ï¼ˆç§�æœ‰å­˜å‚¨åŒºï¼‰ã€‚
  è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆè®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡å®šçš„æ ‡ç­¾è¿›è¡Œæ“�ä½œã€‚

  Read/Write:	0x01(0x00: Readï¼Œ0x01: Write)
  Persistence:	0x01(0x00: å†™å…¥æ ‡ç­¾æŒ¥å�‘æ€§å­˜å‚¨åŒºï¼Œ0x01: å†™å…¥é�žæŒ¥å�‘æ€§å­˜å‚¨åŒº)
  Payload:	0x4000(QT Controlï¼Œæœ€é«˜ä¸¤ä¸ª bits åˆ†åˆ«ä¸º QT_SR å’Œ QT_MEM)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::Impinj_Monza_QT(UDOUBLE Access_Password, UBYTE Read_Write, UBYTE Persistence, UWORD Payload)
{
  UBYTE a[5] = {0xBB, 0x01, 0xE5, 0x00, 0x11};
  UBYTE b[5] = {0xBB, 0x01, 0xE6, 0x00, 0x10};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(37);

  ToHex(Access_Password, 5, 8);
  ToHex(Read_Write, 9, 9);
  ToHex(Persistence, 10, 10);
  ToHex(Payload, 11, 12);

  Check_bit_accumulation();
  Send_the_modified_command();

  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Cardinformation = UI_PC_EPC();
      Cardinformation._Successful = "QT Control :0" + DATA_Str_M5led.substring( 40,  42) + "1" + DATA_Str_M5led.substring( 42,  44) + "Read Impinj Monza QT OK";

      return Cardinformation;
    }
    else if (Verify_the_return(b, 5))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "Write Impinj Monza QT OK";

      return Cardinformation;
    }

  }
return Cardinformation;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Used for the BlockPermalock directive
  The BlockPermalock directive can lock blocks permanently or read the locked state of a Block.
  This directive should be preceded by setting the SELECT parameter to Select the specified label to operate on.
  Read_Lock: 0 x00 (0 x00: Read, 0 x01: Lock)
  Block PTR: 0x0000 Block address of Mask
  Block range: 0x01
  Mask: 0x0700(when Read/Lock data field is 0x00, i.e. Read state, this data field is omitted)

  ç”¨äºŽBlockPermalock æŒ‡ä»¤

  BlockPermalock æŒ‡ä»¤å�¯ä»¥æ°¸ä¹…é”�å®šç”¨æˆ·åŒºçš„æŸ�å‡ ä¸ª Blockï¼Œæˆ–è€…è¯»å�– Block çš„é”�å®šçŠ¶æ€�ã€‚
  è¿™æ�¡æŒ‡ä»¤ä¹‹å‰�åº”å…ˆ è®¾ç½® Select å�‚æ•°ï¼Œä»¥ä¾¿é€‰æ‹©æŒ‡å®šçš„æ ‡ç­¾è¿›è¡Œæ“�ä½œã€‚

  Read_Lock:	0x00(0x00:Readï¼Œ0x01:Lock)
  BlockPtr:	0x0000(Mask çš„èµ·å§‹ Block åœ°å�€ï¼Œä»¥ 16 ä¸ª Block ä¸ºå�•ä½�)
  BlockRange:	0x01(16 ä¸ª Block ä¸ºå�•ä½�)
  Mask:	0x0700(å½“ Read/Lock æ•°æ�®åŸŸä¸º 0x00ï¼Œå�³è¯»å�–çŠ¶æ€�æ—¶ï¼Œè¯¥æ•°æ�®åŸŸçœ�ç•¥)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CardInformationInfo UHF_RFID::BlockPermalock(UDOUBLE Access_Password, UBYTE Read_Lock, UBYTE MemBank, UWORD BlockPtr, UBYTE BlockRange, UWORD Mask)
{
  UBYTE a[5] = {0xBB, 0x01, 0xD3, 0x00, 0x12};
  UBYTE b[5] = {0xBB, 0x01, 0xD4, 0x00, 0x10};
  UBYTE e = 0;

  CardInformationInfo Cardinformation;

  Copy_command_library(38);

  ToHex(Access_Password, 5, 8);
  ToHex(Read_Lock, 9, 9);
  ToHex(MemBank, 10, 10);
  ToHex(BlockPtr, 11, 12);
  ToHex(BlockRange, 13, 13);
  ToHex(Mask, 14, 15);

  Check_bit_accumulation();
  Send_the_modified_command();

  Readcallback();

  if (e == DelayScanwarning())
  {
    if (e == 0x16)
    {
      Return_to_convert(1);
      Cardinformation = Access_Password_is_incorrect();
    }
    else if (e / 0x10 == 0xE )
    {
      Return_to_convert(1);
      Cardinformation = EPC_Gen2_error_code();
    }
    return Cardinformation;
  }
  else
  {

    if (Verify_the_return(a, 5))
    {
      Return_to_convert(1);

      Cardinformation = UI_PC_EPC( );
      Cardinformation._Successful = "BlockPermalock :" + DATA_Str_M5led.substring( 40,  44) + "Read Impinj Monza QT OK";

      return Cardinformation;
    }
    else if (Verify_the_return(b, 5))
    {
      Return_to_convert(1);

      Cardinformation = Operation_is_successful();
      Cardinformation._Successful = "Lock Impinj Monza QT OK";


      return Cardinformation;
    }

  }
return Cardinformation;
}

