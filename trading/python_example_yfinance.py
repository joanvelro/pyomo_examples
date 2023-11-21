import json
import yfinance as yf

stock_obj = yf.Ticker("AMZN")
# Here are some fixs on the JSON it returns
validated = str(stock_obj.info).replace("'", "\""). \
                                replace("None", "\"NULL\"").\
                                replace("False", "\"FALSE\"").\
                                replace("True", "\"TRUE\"")

# Parsing the JSON here
meta_obj = json.loads(validated)

print('\n')
for key in meta_obj.keys():
    print('{}: {}'.format(key, meta_obj[key]))
print('\n')
# Some of the short fields
print("sharesShort: " + str(meta_obj['sharesShort']))
print("shortRatio: " + str(meta_obj['shortRatio']))
print("shortPercentOfFloat: " + str(meta_obj['shortPercentOfFloat']))
