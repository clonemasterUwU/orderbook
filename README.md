# Orderbook

## Brief

This project provides real-time order book simulator using data from market exchanges' feed,
which are foundation of any backtesters, especially those that aim for high frequency trading.

## Progress

- [x] ITCH protocol (NASDAQ) parser
- [ ] Openbook Ultra protocol (NYSE) parser
- [ ] PITCH protocol (CBOE) parser
- [x] aggregate book
- [x] level depth book
- [x] single book simulator
- [ ] parallel book simulator

## Demo
#### Clone the project:
```bash
git clone https://github.com/clonemasterUwU/orderbook.git
cd orderbook
```

#### Pull the data:
Either with `data_pull.py` script:
```bash
#usage python3 data_pull.py [number of file pulled,absent = all]
python3 data_pull.py 1
```
Or with command line:
```bash
wget -P ./data "https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20181228.BX_ITCH_50.gz" | gunzip
```
Or manually through at [this link](https://emi.nasdaq.com/ITCH/).

#### Build system
The project uses CMake build system and comes with 3 targets/executables:
```bash
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=release
cmake --build build --target all
```

The "orderbooksimulator" target runs a command line visualizer:
```bash
#Usage ./build/orderbooksimulator [path-to-input-file] [ticker]
./build/orderbooksimulator ./data/20181228.BX_ITCH_50 MSFT
```

![](https://github.com/clonemasterUwU/orderbook/blob/misc/demo.gif)

The "orderbookaggregate" target consolidates price-volume data for every ticker to a csv file:

```bash
#Usage: ./build/orderbookaggregate [path-to-input-file] [depth_lvl]
./build/orderbookaggregate ./data/20181228.BX_ITCH_50 3
```
![](https://github.com/clonemasterUwU/orderbook/blob/misc/agg.png)

The "orderbookstat" target collects data metric (mean, std, ...) of the number of order per order type, the executed price, ...
for each ticker:

```bash
#Usage ./build/orderbookstat [path-to-input-file]
./build/orderbookstat ./data/20181228.BX_ITCH_50
```

![](https://github.com/clonemasterUwU/orderbook/blob/misc/stat.png)
