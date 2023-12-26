#
# Examples for pyomo.mpec
#

#
# Other Examples
#

See also the pyomo.data.mcplib package.


#
# Generating NL files
#

Here's how to generate NL files from Pyomo

  pyomo convert --nl --symbolic-solver-labels linear1.py

This generates unknown.nl, unknown.col, unknown.row


Here's how to generate NL files from AMPL

  ampl linear1_run.mod

This generates ampl.nl, ampl.col, ampl.row


