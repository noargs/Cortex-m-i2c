### Interfacing DS1307 RTC with Nucleo

> :warning: While interfacing with DS1307 Real Time Clock (Slave), the I2C bus busy flag will be set in STM32 (ST MCUs I2C Analog filters bug). To fix this behaviour in ST's microcontrollers follow the link below. _2.9.7 I2C analog filter may provide wrong value, locking BUSY flag and preventing master mode entry_
- [Errata sheet](https://www.st.com/content/ccc/resource/technical/document/errata_sheet/7f/05/b0/bc/34/2f/4c/21/CD00288116.pdf/files/CD00288116.pdf/jcr:content/translations/en.CD00288116.pdf) for workaround