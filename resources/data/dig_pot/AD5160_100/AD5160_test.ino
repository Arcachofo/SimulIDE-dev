#include <SPI.h>

enum { REG_SELECT = 8 }; // пин, управляющий защёлкой (SS в терминах SPI)

void setup()
{
  /* Инициализируем шину SPI. Если используется программная реализация,
   * то вы должны сами настроить пины, по которым будет работать SPI.
   */
  SPI.begin();

  pinMode(REG_SELECT, OUTPUT);
  digitalWrite(REG_SELECT, LOW); // выбор ведомого - нашего регистра
  SPI.transfer(0); // очищаем содержимое регистра
  /* Завершаем передачу данных. После этого регистр установит
   * на выводах QA-QH уровни, соответствующие записанным битам.
   */
  digitalWrite(REG_SELECT, HIGH);
}


void loop()
{
 for (byte i=0;i<=255;i++){

  /* Записываем значение в сдвиговый регистр */
  digitalWrite(REG_SELECT, LOW);
  SPI.transfer(i);
  digitalWrite(REG_SELECT, HIGH);

  delay(100);
}
}
