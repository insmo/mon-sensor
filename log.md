## 2010.10.14

_hardware_: Pot, led, external osc on uC connected to 3.7V batt batt. 2.4V to Xbee,
I draw from same batt.

_software_: no sleep, continous wifi transmittion. free-running adc.

date           mod              I                    V
----------  ------  -------------        -------------
14.10         Xbee    30.0 - 32.0          2.40 - 2.54
14.10         328P    30.0 - 33.0          3.3


By turning off the TX and put the mote into sleep-mode when there arn't any adc
interrupts, we can lower the current usage.

date           mod              I                    V
----------  ------  -------------        -------------
14.10         Xbee    30.0 - 32.0          2.40 - 2.54
14.10         328P    14.0 - 18.0          3.3

SUM
----------  ------  -------------        -------------
sum         --|--   44.0 - 50.0          3.3

## 2010.10.15

Experiment with power modes on ATmega328P

_setup_: atmega328p on breadboard, external osc, powered by 3.3v voltage.
The current was mesured at VIN for the entire application.

_power modes versus current draw_ 

mode                    I
---------------------   ------------
idle                    2.95
ADC noice reduction     2.08 - 2.09
power-down              0.00
power-save              1.97
standby                 1.21
extended standby        1.97


## 20.10.17

Battery duration calculations based on _test/sleep/sleep_test.c_. In this test
we don't take the photovoltanic charges into account. The goal of the experiment
is to find the how long the module can operate only running on it's battery
power.


mode                    I           time-running
---------------------   ---------   ------------
transmitting            61.5 mAh    ~0.150 s
power-save              2.66 mAh    10 s

Table 1: show's the current draw vs. time in seconds.

Based on table 1 we can get the executing time of each mode by getting the
multiplication factor

    5.91133005 = 3600(sec h) / 10.150 / 60

5.91133005 * 10 = 59.1133005 s sleep * 60(min) = 3547 sec per hour
The application will sleep for 3547 seconds per hour.

Using the 98.3 % sleep time an average mAh usage can be calculated:

    avg mAh = mAh * per + (mAh * per) 
    3.66028 mAh = 2.66 * 0.983 + (61.5 * 0.017)

Since the current voltage regulator must have minimum of 200mV dropout voltage
at ~200 mAh we can lower the available capacity of energy source. 

Battery used for testing is a lithium Ion Polymer battery with an 1200 mAh (4.5 Wh)
storage time measured in amp-hours or milliamp-hours.

Battery has a nominal voltage of 3.7V, charge cut-off voltage at 4.2V and discharge
cut-off voltage at 3.0V. This means we need at least 3.2V to operate. 

    voltage operation range = charge cut-off - discharge cut-off
    1.2V = 4.2 - 3.0
    16 % loss = (0.2V / 1.2V) * 100 
    3.2V lowest operation = (4.2 - (3.0 - 0.2)

We can use the average current consumption and add the loss to get more accurate
results.

    4.35 mAh avg = 3.66028 / 0.84
    11.49 days of operation = 1200 / 4.35 / 24 h

By increasing the sleep-mode time and increasing the sleep time we can reduce
the I consumption further.

One update per hour gives us 99.75 % sleep time and 0.25% uptime
    
#. 2.8071 mAh = 2.66 * 0.9975 + (61.5 * 0.0025)
#. 3.34178571 mAh = 2.8071 / 0.84
#. 14.96 days  = (1 200 / 3.34178571) / 24 

We gain a few days, a better solution were if sleep mode would consume less
power. By using a different sleep mode in which we only consume 1.5 picowatt we
are able to increase uptime to ~ 160 days.

One update per hour using a watchtimer interrupt 
#. 0.63255mAh = 0.48 * 0.9975 + (61.5 * 0.0025)
#. 0.753035714 = 0.63255 / 0.84
#. 66 days = 1200 / 0.753035714 / 24

One update per hour using a watchtimer interrupt using the MAXIM 882 linear low
dropout power regulator.
#. 0.163725 = 0.01 * 0.9975 + (61.5 * 0.0025)
#. 


(We can get the watt hours by multiplying the nominal voltage with the storage
capacity.

_E = C * Vavg_

4.5 = 1.2 * 3.7 
E = 4.5 Wh (or 16200 Joules))

If current draw is _x_ amps, time is _T_ hours then capacity _C_ in amp-hours is

    C = xT
    C = 0.03 A * 24 h = 0.72 amp hours

### Formulas

One amp flowing for one second will use one coulomb of charge.

_Q = I * t_

Q is coulomb, I and time


## Timers 

Timers can run asynchronous, communicating via count registers and interrupts.
The smallest time a timer can measure is one period of the incoming clock signal. 

_(examples with 1Mhz)_:

    resolution = (1 / frequency)
    
    resolution = (1 / 100)
    resolution = 0.01 sec

    timer count = (1 / frequency) / (1 / timer clock frequency) - 1
    timer count = (1 / 20) / (1 / 1000000) - 1 
    timer count = 0.05 / 0.000001 - 1
    timer count = 49999

We can set the timer prescaler to trade of resolution to duration.

    resolution = (1 / (frequency / prescale))

    timer count = (1 / target frequency) / (prescale / input frequency) - 1
    # rearanged
    timer count = (timer clock frequency / prescaler) / target frequency - 1

We can extend this technique to create our own prescaler which ticks seconds,
minutes etc.

In stead of doing the calculations our self it can be made by the uC's hardware.
This is done using something called _Clear on Timer Compare_(CTC). We still need
to configure the timer at a certain prescaler, and we have to set a compare
value, which will set interrupt flag the timer -- but the comparison is done at hardware
level.

ctc timer:

    (enable timer)
    (set prescaler clock)
    (set compare value)
    (check CTC flag)
    (clear ctc event flag)

Another approach with interrupts:

    (enable timer)
    (set prescaler clock)
    (set compare value)
    (enable ctc interrupts)
    (enable global interrupts)

    on timer interrupt()
        tick time var
    end

A slightly different approach is to use the built in knowledge of bits maximum
storage capability in something called a timer overflow, simply a timer which
runs out of space - and starts counting at the beginning again. 

We need to calculate the frequency rom the timer count and the timer resolution.

    (timer count + 1) * (prescale / input frequency) = (1 / target frequency) 
    # swapped
    (1 / target frequency = (target timer count + 1) * (prescaler / input
    frequency)
    target period = ((2 ^ bit) * (prescale / input frequency)


prescaler   overflow frequency      overflow period
---------   ------------------      ----------------------------
perscale    (1 / overflow period)   ((2 ^ 16) * (64 / 1000000))

64          0.23841 Hz              4.194304 s
