EEPROM 驱动模板

I2C : drivers/misc/eeprom/at24.c
SPI : drivers/misc/eeprom/at25.c


---------------------
dts for imx6q

```
&ecspi5 {
	fsl,spi-num-chipselects = <1>;
	cs-gpios = <&gpio1 17 0>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_ecspi5_1 &pinctrl_ecspi5_cs_0>;
	status = "okay";

	at25@0 {
		compatible = "st,m95080";
		spi-max-frequency = <10000000>;
		pagesize = <32>;
		size = <1024>;
		address-width = <16>;
		spi-cpha;
		spi-cpol;
		reg = <0>;
	};
};
```

