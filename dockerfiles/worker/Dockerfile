FROM python:3.7

# Instalando atualizações e rsync
RUN apt-get update -y
RUN apt-get install rsync -y

# Criando estrutura de diretórios
RUN mkdir /scripts
ADD main.py /scripts

RUN mkdir /scripts/Treinamento
ADD Treinamento/ /scripts/Treinamento

RUN mkdir /scripts/Validacao
ADD Validacao/ /scripts/Validacao

RUN mkdir /scripts/Mask_RCNN
ADD Mask_RCNN/ /scripts/Mask_RCNN

RUN mkdir /scripts/logs
ADD logs/ /scripts/logs

# Instalando módulos
WORKDIR /scripts/Mask_RCNN
RUN python setup.py install

RUN /usr/local/bin/python -m pip install --upgrade pip
RUN pip3 install --default-timeout=300 protobuf==3.20.* requests numpy scipy Pillow cython matplotlib scikit-image==0.16.2 tensorflow-gpu==1.15.0 tensorflow==1.15.5 keras==2.1.6 opencv-python h5py imgaug IPython[all]

# Configurando usuário SSH
RUN adduser --disabled-password --gecos "" ec2-user
RUN mkdir /home/ec2-user/.ssh
RUN chmod 600 /home/ec2-user/.ssh
RUN echo "-----BEGIN RSA PRIVATE KEY-----\n\
MIIEpAIBAAKCAQEAtUErQJH9rCh5OWCdlCiHbbjZMFlzOu+gPPb6ocmf3hXaAa+E\n\
aY8q4l5GGISouB7VkvH/5DOhz0Yg1EKv1yUzgozVMYbZBCqVEITVQCpgc/hHzBLT\n\
7CmJu7sFixuG+YigSo6/CVxx6SBnljy+tfzAumyggix3K2syrYFcIb79TnsceMly\n\
Om5gfGErGsHa9kEt04CaNyq65ngP8ogDn6icARMK1vLb1LB//JU0dS0sFJkSfhtU\n\
McA6ErUjVbXlj1+wkenuk4je3x2DFWY/Xp4kTYwHifIMU1ysEgKyGdEFEa33VfnD\n\
RwHytKRYoAvSAZ6AHfKrFSYKqrFML7AnG6UoBwIDAQABAoIBAGTUziTrkjz1fwqG\n\
TuMe8Mf/LwOQn2PH7lI76w2Zxo+bhF2vsG+wr2CipUf6dImIzbMjYQEoH/nYv2aV\n\
epCrJ06W3frhU1TYA3jfDGrXqDSbiAVk6/PS38ZJsjnCpIGomdMgOfIRNWVFG5se\n\
S9Gdu/F5gIkqB1lmFxkif4heYTgIoDlcKt8DW92BVz6qpz+9xotbEVV/dnnY8BGw\n\
biVN9Xtvqrk/WOvYy9T8SqY91MP82C853H5eLiR8oZBQzK/VNr5TFeQcSJoARX+S\n\
PWkDr/kzCZwq3L1EY9dYrn3HgmOTKhzlXwcEo55pqFS8CtMD6mZ5pwY+b4WGSQ7L\n\
3aLqDrECgYEA6kRB2W60KOB8eSjKin7c9YgomL2V8ba2AG79C9H0a4ee4blx1BzF\n\
HY/ldofqPXySCODprko2VMPAjHOwDbkYXBGB6qLG1tLr2y4w8RyWa+rhE4PUoKRi\n\
8qn5vUqsDljBOHxEeZEEBIUTO+8nG7wQvee9alon5UHEl17iiVBRozMCgYEAxhHl\n\
QHCDTP3BIaz4hc0QZFOz0VRjZRbDvxT9C4ItSAs0KboMC8+Oh93AE75EscXBjH/e\n\
5C+el6SS1PLi90avGAiEhbVlBBXZjNQKZU8DQvJssUQyu337zxdr2ia4UlW8/+ZW\n\
QXbwTb6s/1K6jmw14M9Wrhad4RL+U67MGL13p90CgYEAmHWBkVrdfrjR6Rvl7Iox\n\
jM1m9jhzzBIHknKo1jd2ZUCm4uoILW3nzpxmTmSXyho3A/Yzvkzn3PRkbEOh8Nxk\n\
AQ8XyLbUxtwPyN3f1hqvvl3myweBqmYxSxHZyRo+rTuPzA37lk4NGk/su5Whkwgq\n\
sRyPBhbUXfwWUBjdP1X5O0cCgYAz1Cj2BLsG4XuxES7y7Cb26cQq6Xcc06kjDh3s\n\
7gJlULE0nf9a89PrD4acY/RbxFCSMD+l3CQWsDTPzkK/y9ozYr8ysbWeLQ2BQVb7\n\
DYLf+MmiVSMlVqEWnp1VNz6SJOdQ2vgFJ1dYMKXaJc61R/GNQxP+d+KWK78rLJkx\n\
fBqcbQKBgQDWOTsx6bvUNBxNGxwDLZj6c0ACOPTGs0Fc8WJma/ZBEHxehzbEnn+W\n\
SGkxgSr2FOqUm1za3KpzDK5W9WNIj55ssjA2ppm30cIidkO15+NUYvwmTgu5TUBs\n\
PEIbmE5Vqcgr1l/hQmHdCANziT+sZUC0zPnOVlFUYFGgrQtcDZELFA==\n\
-----END RSA PRIVATE KEY-----" > /home/ec2-user/.ssh/default-key.pem
RUN chmod 700 /home/ec2-user/.ssh/default-key.pem

# Comando de início do programa
CMD [ "python3", "/scripts/main.py" ]