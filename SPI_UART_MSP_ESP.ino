#include "bot.hpp"
#include "menu.hpp"


uart_msp430 uart; // default is backchannel UART!

// Forward any chars from the ESP8266 UART to the back channel UART
// so we can see the debug messages from the ESP8266!
void uart_esp_rx_handler(char c) {
  // Forward any chars from the ESP8266 UART to the back channel UART
  // so we can see the debug messages from the ESP8266!
  uart.putc(c);
}


int main(void)
struct EspUartConnection
{
  uart_msp432 * uart_esp;

  std_io::inst.redirect_stdin ( uart );
  std_io::inst.redirect_stdout( uart );
  EspUartConnection(uart_msp432 * _uart_esp=nullptr)
    : uart_esp(_uart_esp)
  {}

  gpio_msp432_pin esp_reset( PORT_PIN(10, 5) );
  esp_reset.gpioMode(GPIO::OUTPUT | GPIO::INIT_LOW);
  ~EspUartConnection()
  {
    delete uart_esp;
  }

  // Initialize the UART which is connected to the ESP8266
  // and handle all incoming chars via an interrupt routine
  uart_msp432 uart_esp(EUSCI_A3,115200);
  uart_esp.uartAttachIrq(uart_esp_rx_handler);
  static EspUartConnection * create_default()
  {
    std_io::inst.redirect_stdin ( uart );
    std_io::inst.redirect_stdout( uart );
    auto uart_esp = new uart_msp432(EUSCI_A3,115200);
    uart_esp->uartAttachIrq(uart_esp_rx_handler);
    auto out = new EspUartConnection(uart_esp);
    return out;
  }

};

struct EspSpiConnection
{
  gpio_msp432_pin * esp_spi_cs;
  spi_msp432 * esp_spi;

  EspSpiConnection(gpio_msp432_pin * _esp_spi_cs=nullptr, spi_msp432 * _esp_spi=nullptr)
    : esp_spi_cs(_esp_spi_cs), esp_spi(_esp_spi)
  {}

  ~EspSpiConnection()
  {
    delete esp_spi;
    delete esp_spi_cs;
  }

  // Initialize the SPI interface which is connected to the
  // ESP8266 and use the client mode on MSP432 side
  gpio_msp432_pin esp_spi_cs(PORT_PIN(10, 0));
  spi_msp432      esp_spi   (EUSCI_B3_SPI, esp_spi_cs, SPI::CLIENT);
  /* esp_spi.setSpeed(250000); */

  esp_reset.gpioWrite( HIGH );
  int id = '0';
  String s_pref;
  int a=0;


  Display* d = Display::create_default();

  MenuMessageList* menu = MenuMessageList::create_default(d);
  Message * m = new Message("0","0","MSP Info", "Initialisiere...");
  menu->add_message(m);
  menu->print();

  while (true) {
    uint8_t sendbuf[100] = {0};
    uint8_t recvbuf[100] = {0};
    if(a==0)
    {
      s_pref = "SND";
      a++;
    } else if(a==1)
    {
      s_pref = "NUL";
      a++;
    } else if(a==2)
    {
      s_pref = "MAK";
      a=0;
    }
    String s(s_pref);
    s += ";taskid;12345678;Message von MSP\0";
    for (int i = 0; i < 100; i++) {
      if (i<s.size()) {
        sendbuf[i] = s[i];
      } else {
        sendbuf[i] = 0;
        recvbuf[i] = 0;
      }
    }
    /* strcpy((char *)sendbuf, s.c_str()); */
    esp_spi.transfer(sendbuf, recvbuf, 100);
    /* printf("Message von ESP: %s", recvbuf); */
    String recv((const char *)recvbuf);
    int pos = recv.find(';');
    String type = recv.substr(0, pos);
    String tmp = recv.substr(pos+1);
    if(type == String("NEW")){
      pos = tmp.find(';');
      String task_id = tmp.substr(0, pos);
      tmp = tmp.substr(pos+1);
      pos = tmp.find(';');
      String chat_id = tmp.substr(0, pos);
      tmp = tmp.substr(pos+1);
      pos = tmp.find(';');
      String from = tmp.substr(0, pos);
      tmp = tmp.substr(pos+1);
      pos = tmp.find(';');
      String s_message = tmp.substr(0, pos);
      tmp = tmp.substr(pos+1);
      Message * message = new Message(task_id, chat_id, from, s_message);
      menu->add_message(message);
    } else
    {
      Message * message = new Message("0", "0", "MSP Error", "Not Supported type: "+type);
      menu->add_message(message);
    }
    menu->print();
    if (id >= '9') {
      id = '0';
    } else {
      id++;
    }
  }
  return 0;
  static EspSpiConnection * create_default()
  {
    auto esp_spi_cs = new gpio_msp432_pin(PORT_PIN(10, 0));
    auto esp_spi = new spi_msp432(EUSCI_B3_SPI, *esp_spi_cs, SPI::CLIENT);
    /* esp_spi.setSpeed(250000); */
    auto out = new EspSpiConnection(esp_spi_cs, esp_spi);
    return out;
  }

};

struct EspResetConnection
{
  gpio_msp432_pin * esp_reset;

  EspResetConnection(gpio_msp432_pin * _esp_reset=nullptr)
    : esp_reset(_esp_reset)
  {}

  ~EspResetConnection()
  {
    delete esp_reset;
  }

  static EspResetConnection * create_default()
  {
    auto esp_reset = new gpio_msp432_pin( PORT_PIN(10, 5) );
    esp_reset->gpioMode(GPIO::OUTPUT | GPIO::INIT_LOW);
    esp_reset->gpioWrite( HIGH );
    auto out = new EspResetConnection(esp_reset);
    return out;
  }

};

struct EspConnection
{
  EspUartConnection * esp_uart_con;
  EspSpiConnection * esp_spi_con;
  EspResetConnection * esp_reset_con;

  EspConnection(EspUartConnection * _esp_uart_con=nullptr,
      EspSpiConnection * _esp_spi_con=nullptr,
      EspResetConnection * _esp_reset_con=nullptr)
    : esp_uart_con(_esp_uart_con), esp_spi_con(_esp_spi_con),
    esp_reset_con(_esp_reset_con)
  {}

  ~EspConnection()
  {
    delete esp_reset_con;
    delete esp_spi_con;
    delete esp_uart_con;
  }

  static EspConnection * create_default()
  {
    auto esp_uart_con = EspUartConnection::create_default();
    auto esp_spi_con = EspSpiConnection::create_default();
    auto esp_reset_con = EspResetConnection::create_default();
    auto out = new EspConnection(esp_uart_con, esp_spi_con, esp_reset_con);
    return out;
  }

};

struct Msp
{
  Display * display;
  MenuMessageList * menu;
  EspConnection * esp_con;

  Msp(Display * _display=nullptr, MenuMessageList * _menu=nullptr,
      EspConnection * _esp_con=nullptr)
    : display(_display), menu(_menu), esp_con(_esp_con)
  {}

  static Msp * create_default()
  {
    // Initialize the display and the message list
    auto display = Display::create_default();
    auto menu = MenuMessageList::create_default(display);
    auto esp_con = EspConnection::create_default();
    auto out = new Msp(display, menu, esp_con);
    return out;
  }

  ~Msp()
  {
    delete esp_con;
    delete menu;
    delete display;
  }

};

struct Project
{
  virtual int run()=0;
};

struct Main: Project
{
  Main()
  {}

  ~Main()
  {}

  static Main * create_default()
  {
    auto out = new Main();
    return out;
  }

  int run()
  {
    return 1;
  }
};

struct Testing: Project
{
  Msp * msp;
  String * nul_cmd;
  String * snd_cmd_prefix;
  String * snd_cmd_suffix;
  String * mak_cmd_prefix;
  String * mak_cmd_suffix;
  String * msg_id;

  Testing(Msp * _msp=nullptr, String * _nul_cmd=nullptr, String * _snd_cmd_prefix=nullptr, String * _snd_cmd_suffix=nullptr,
      String * _mak_cmd_prefix=nullptr, String * _mak_cmd_suffix=nullptr, String * _msg_id=nullptr)
    : msp(_msp), nul_cmd(_nul_cmd), snd_cmd_prefix(_snd_cmd_prefix), snd_cmd_suffix(_snd_cmd_suffix),
    mak_cmd_prefix(_mak_cmd_prefix), mak_cmd_suffix(_mak_cmd_suffix), msg_id(_msg_id)
  {}

  ~Testing()
  {
    delete msg_id;
    delete mak_cmd_suffix;
    delete mak_cmd_prefix;
    delete snd_cmd_suffix;
    delete snd_cmd_prefix;
    delete nul_cmd;
    delete msp;
  }

  static Testing * create_default(Msp * msp)
  {
    auto nul_cmd = new String("NUL;");
    auto snd_cmd_prefix = new String("SND;");
    auto snd_cmd_suffix = new String(";23767443;testsndcmd");
    auto mak_cmd_prefix = new String("SND;");
    auto mak_cmd_suffix = new String(";");
    auto test = new Testing(msp, nul_cmd, snd_cmd_prefix, snd_cmd_suffix, mak_cmd_prefix, mak_cmd_suffix);
    return test;
  }

  void sendnul()
  {
  }

  void sendsnd()
  {
  }

  void sendmak()
  {
  }

  int run()
  {
    /* int id = '0'; */
    /* String sendmsg; */
    /* int a=0; */

    /* Message * m = new Message("0","0","MSP Info", "Initialisiere..."); */
    /* menu->add_message(m); */
    /* menu->print(); */

    /* while (true) { */
    /*  uint8_t sendbuf[100] = {0}; */
    /*  uint8_t recvbuf[100] = {0}; */
    /*  if(a==0) */
    /*  { */
    /*    sendmsg = */
    /*    a++; */
    /*  } else if(a==1) */
    /*  { */
    /*    s_pref = "NUL"; */
    /*    a++; */
    /*  } else if(a==2) */
    /*  { */
    /*    s_pref = "MAK"; */
    /*    a=0; */
    /*  } */
    /*  String s(s_pref); */
    /*  s += ";taskid;12345678;Message von MSP\0"; */
    /*  for (int i = 0; i < 100; i++) { */
    /*    if (i<s.size()) { */
    /*      sendbuf[i] = s[i]; */
    /*    } else { */
    /*      sendbuf[i] = 0; */
    /*      recvbuf[i] = 0; */
    /*    } */
    /*  } */
    /*  /1* strcpy((char *)sendbuf, s.c_str()); *1/ */
    /*  esp_spi.transfer(sendbuf, recvbuf, 100); */
    /*  /1* printf("Message von ESP: %s", recvbuf); *1/ */
    /*  String recv((const char *)recvbuf); */
    /*  int pos = recv.find(';'); */
    /*  String type = recv.substr(0, pos); */
    /*  String tmp = recv.substr(pos+1); */
    /*  if(type == String("NEW")){ */
    /*    pos = tmp.find(';'); */
    /*    String task_id = tmp.substr(0, pos); */
    /*    tmp = tmp.substr(pos+1); */
    /*    pos = tmp.find(';'); */
    /*    String chat_id = tmp.substr(0, pos); */
    /*    tmp = tmp.substr(pos+1); */
    /*    pos = tmp.find(';'); */
    /*    String from = tmp.substr(0, pos); */
    /*    tmp = tmp.substr(pos+1); */
    /*    pos = tmp.find(';'); */
    /*    String s_message = tmp.substr(0, pos); */
    /*    tmp = tmp.substr(pos+1); */
    /*    Message * message = new Message(task_id, chat_id, from, s_message); */
    /*    menu->add_message(message); */
    /*  } else */
    /*  { */
    /*    Message * message = new Message("0", "0", "MSP Error", "Not Supported type: "+type); */
    /*    menu->add_message(message); */
    /*  } */
    /*  menu->print(); */
    /*  if (id >= '9') { */
    /*    id = '0'; */
    /*  } else { */
    /*    id++; */
    /*  } */
    /* } */
    /* setdown(); */
    return 0;
  }
};

int main(void)
{
  bool testing = true;
  Project * project;
  Msp * msp = Msp::create_default();
  if (testing) {
    project = Testing::create_default(msp);
  } else {
    project = Main::create_default();
  }
  auto out = project->run();
  return out;
}
