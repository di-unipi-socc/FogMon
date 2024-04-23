import requests

response = requests.get('http://localhost:8080/api/testbed')
print(response.status_code)
sessions = response.json()["data"]
print(sessions)
sessions = [el["_id"]["session"] for el in sessions]
if 0 not in sessions:

    # add testbed 0
    # nodes should be ips of the nodes or names resolvable by dns (how inside docker?)
    spec = {"monitor": True, "nodes": [""]}

    response = requests.post('http://localhost:8080/api/testbed', json=spec)
    print(response.status_code)
    print(response.json())

    extra = {"extra": "extra"}
    response = requests.put('http://localhost:8080/api/testbed/0/extra', json=extra)
    print(response.status_code)
    print(response.json())

response = requests.get('http://localhost:8080/api/testbed/0/monitor')
print(response.status_code)
print(response.json())