# Overview of Printed Circuit Boards (PCBs)

The PCB designs presented in this project are intended to make your life easier.
PCBs have the potential to be **cheaper** than manufacturing with punched boards
and eliminate **a lot** of soldering work.
Optionally, manual assembly of components can also be eliminated,
but is more costly.

There are a number of companies that provide this service **online**,
both locally and globally.
All you have to do is upload some files,
pay and wait for the postman.
To name some:

- [JLC PCB](https://jlcpcb.com/?href=easyeda-home)
- [PCB way](https://www.pcbway.com)
- [OSH Park](https://oshpark.com/)
- [Seed Studio Fusion](https://www.seeedstudio.com/fusion_pcb.html)
- [All PCB](https://www.allpcb.com/)
- [PCB Cart](https://www.pcbcart.com/quote/prototype-pcb)
- [Elecrow](https://www.elecrow.com/pcb-manufacturing.html)
- [PCB Gogo](https://www.pcbgogo.com/pcb-fabrication-quote.html)
- [Proto-electronics](https://www.proto-electronics.com/)
- [Euro circuits](https://www.eurocircuits.com/)

[PCB shopper](https://pcbshopper.com/) provides a price comparison among several online services.

A **minimum order** is required, which is usually five units.
Optionally, you can order component assembly and soldering service,
but please note the following:

- The cost of the components is significantly higher than that of the PCB itself
  and is multiplied by the minimum order.

- Surface-mount components are not suitable for mounting with household tools.
  You will need to order assembly on designs that include these components.
  Their advantage lies in smaller board sizes.

- The designs include components that may not be available from your chosen supplier,
  so you will have to do the work of substituting one for another.

To keep costs down, this project incorporates designs featuring through-hole components,
eliminating the need for assembly services.
You can obtain the components from a local supplier or buy them online.
Then all you have to do is solder them to the printed circuit board yourself.

## Constraints

Each supplier of printed circuit boards imposes design rules
that depend on its manufacturing technology.
The designs included in this project are based on the
[JCL PCB](https://jlcpcb.com/capabilities/pcb-capabilities) manufacturing capabilities.
If your supplier does not accept them,
you will need to edit the designs,
adjust the constraints and redesign the traces.
