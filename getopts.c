/* �� ��⨢�� ���⠢襣� ᢮�� �㯮���� ���� getopt().

�����頥�:

OptNum - ������ �����㦥���� ������� � ���ᨢ� 㪠��⥫��
         ��ப���� ����⠭� OptArr[OptNum]
   ���
       - ������ argv[OptNum] �������⭮� �������.

ParamNum - ������ �����㦥����� ��ࠬ��� ������� � ���ᨢ�
           㪠��⥫�� ��㬥�⮢ ��������� ��ப� argv[ParamNum].

return:
        0  - �����㦥�� �������;
        1  - �����㦥�� ������� � ��ࠬ��஬;
       ':' - �����㦥�� ������� � ���������騬 ��ࠬ��஬;
       '?' - �����㦥� ��ࠬ��� ��� ������� ��� ������������ �������;
       -1  - ���௠� ᯨ᮪ ��㬥�⮢ argv[].

�ਬ�� ������� ���ᨢ� ������ � �ਧ����� ��ࠬ��஢ ������:

char *OptArr[] = {
  "-a",                // ॣ���஭�����ᨬ�� ������� 'a' ��� ��ࠬ���.
  "-b", "^",           // ॣ���஧���ᨬ�� ������� 'b' ��� ��ࠬ���.
  "-cmd1", ":",        // ॣ���஭�����ᨬ�� ������� 'cmd1' � ��ࠬ��஬.
  "-cmd2", "^", ":",   // ॣ���஧���ᨬ�� ������� 'cmd2' � ��ࠬ��஬.
  ""                   // �ਧ��� ����砭�� ���ᨢ�.
};

*/


#include <os2.h>
#include <stdio.h>
#include <string.h>

int         OptNum, ParamNum;
//static int  ArgIdx = 0;       // ������ ��ॡ�� ���祭�� argv[].
int  ArgIdx = 0;       // ������ ��ॡ�� ���祭�� argv[].


int getopts(int argc, char **argv, char **OptArr)
{

  int  OptIdx = 0;    // ������ ��ॡ�� ���祭�� OptArr[]
  UCHAR  f_Reg = 0;   // �ਧ��� ॣ���஧���ᨬ���.

  if(++ArgIdx >= argc) return -1;

  while(*OptArr[OptIdx] != '\0') {
    if(!stricmp(OptArr[OptIdx], argv[ArgIdx])
      && *argv[ArgIdx] != ':' && *argv[ArgIdx] != '^') {

      /* �����㦥�� ������� (ᮢ�������) */

      if(*OptArr[OptIdx + 1] == '^') f_Reg = 1;
      if(!f_Reg || (f_Reg && !strcmp(OptArr[OptIdx], argv[ArgIdx]))) {

        /* �� �ॡ���� ॣ���஧���ᨬ���� ��� ᮢ������� ॣ���஧���ᨬ� */

	OptNum = OptIdx++;			// �����㦥�� ������� (ᮢ�������).
	if(f_Reg) ++OptIdx;                     // �ய����� �ਧ��� ॣ���஧���ᨬ���.
        if(*OptArr[OptIdx] == '\0')
          return 0;				// ᯨ᮪ ������ � �ਧ����� ���௠�.
        if(*OptArr[OptIdx] != ':')
	  return 0;                             // ������� ��� ��ࠬ���.

        /* ������� �ॡ�� ��ࠬ��� */

        if(++ArgIdx >= argc)
          return ':';				// ������� � ���������騬 ��ࠬ��஬.
        ParamNum = ArgIdx;			// ��ࠬ��� �������.
        return 1;				// ������� � ��ࠬ��஬.
      }
    }
    OptIdx++;                                   // ᮢ������� �� �����㦥��.
  }
  OptNum = ArgIdx;
  return '?';                                   // ��ࠬ��� ��� ������� ���
                                                // ������������ �������.
}

