import json
import pandas

list = ['a', 'b', 'c']
A = {'graph': {(i, j): 4 for i in list for j in list}}
A = {'graph': 2}
A_df = pandas.DataFrame.from_dict(A, orient="index")
A_json = A_df.to_json(orient='index')

with open('json_example.json', 'w') as fp:
    json.dump(A, fp, indent=2)
