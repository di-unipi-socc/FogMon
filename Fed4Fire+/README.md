#### Fed4Fire+ testbed

To simplify the deployment of multiple node we created: a script to create the xml rSpec to feed into the rSpec editor of the jFed Experimenter Toolkit, a scipt that setup the fabric folder to run all the commands inside the nodes of the testbed.

### create_spec.py

To create an xml spec you should edit the create_spec.py to match the desired deployment, then run it. The script will output to the stdout the rSpec that must be copied inside the rSpec Editor, deleting all the previus code inside. The script will also create a spec.json file that is necessary to the load_exp.py script.

### load_exp.py

To execute commands (setup of the networks, install docker) inside the nodes of the testbed, we must first download the deployment zip via "Export As" -> "Export Configuration Management Settings (Ansible, Fabric, ...)". Then we must feed the spec.json (previusly generated) and the deployment zip to the load_exp.py as arguments.

The result is a folder "build" where we can run fabric commands (install fabric 2+) via "fab command" the available commands for now are: setupNetwork that create all the tunnels that are not created by jFed, removeNetwork to remove the previusly create network via setupNetwork, setupDocker that will install docker and enable the NAT, deployFogmon to deply fogmon.
