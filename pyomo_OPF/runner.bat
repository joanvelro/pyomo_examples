set parent=%~dp0%
CD "%parent%"

pyomo MODEL.py Model_data.dat --solver=ipopt --solver-options="max_iter=1000000" --summary --stream-solver --report-timing > results.py
