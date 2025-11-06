//
// Created by aoao on 25-3-2.
//


/*
  Serial port access for NTRIP client for POSIX.
  $Id$
  Copyright (C) 2008 by Dirk Stöcker <soft@dstoecker.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  or read http://www.gnu.org/licenses/gpl.txt
*/

/* system includes */
#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "serial.h"
#include <mutex>
using namespace std;

mutex g_smtx;

void SerialFree(struct serial *sn)
{
  if(sn->Stream)
  {
    /* reset old settings */
    tcsetattr(sn->Stream, TCSANOW, &sn->Termios);
    close(sn->Stream);
    sn->Stream = 0;
  }
}

const char * SerialInit(struct serial *sn,
const char *Device, enum SerialBaud Baud, enum SerialStopbits StopBits,
enum SerialProtocol Protocol, enum SerialParity Parity,
enum SerialDatabits DataBits, int dowrite
#ifdef __GNUC__
__attribute__((__unused__))
#endif /* __GNUC__ */
)
{

  struct termios newtermios;
  g_smtx.lock();
  sn->Stream = open(Device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  g_smtx.unlock();

  if(sn->Stream < 0)
  {
    sn->isOpen = false;
    return "could not open serial port";
  }

  tcgetattr(sn->Stream, &sn->Termios);

  memset(&newtermios, 0, sizeof(struct termios));
  newtermios.c_cflag =
    static_cast<unsigned int>(Baud) | static_cast<unsigned int>(StopBits)| static_cast<unsigned int>(Parity) | static_cast<unsigned int>(DataBits) | CLOCAL | CREAD;
  if(Protocol == SPAPROTOCOL_RTS_CTS)
    newtermios.c_cflag |= CRTSCTS;
  else
    newtermios.c_cflag |= Protocol;
  newtermios.c_cc[VMIN] = 1;
  tcflush(sn->Stream, TCIOFLUSH);
  tcsetattr(sn->Stream, TCSANOW, &newtermios);
  tcflush(sn->Stream, TCIOFLUSH);
  fcntl(sn->Stream, F_SETFL, O_NONBLOCK);
  sn->isOpen = true;
  return 0;
}

int SerialRead(struct serial *sn, char *buffer, size_t size)
{
  if(!sn->isOpen) {
    return -110;
  }
  g_smtx.lock();
  int j = read(sn->Stream, buffer, size);
  g_smtx.unlock();
  if(j < 0)
  {
    if(errno == EAGAIN)
      return 0;
    else
      return j;
  }
  return j;
}

void printHexString(const char *buffer, size_t size)
{
  for(size_t i = 0; i < size; ++i) {
    printf("%08X ", buffer[i]);
  }
  printf("\n");
}

int SerialWrite(struct serial *sn, const char *buffer, size_t size)
{
  if(!sn->isOpen) {
    return -110;
  }
  g_smtx.lock();
  int j = write(sn->Stream, buffer, size);
  //printHexString(buffer, size);
  //printf("--SerialWrite : %d %s , ret:%d \n", size, buffer, j);
  g_smtx.unlock();
  if(j < 0)
  {
    if(errno == EAGAIN)
      return 0;
    else
      return j;
  }
  return j;
}


