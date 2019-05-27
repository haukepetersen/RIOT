

# TODO
3.3.1 stateless address autoconfiguration:
- verify SAAC as in RFC7668, 3.2.2
- multihop DAD MUST be supported
3.3.2:
- route-over in neighbor discovery (RFC6775 and RFC8505) MUST be supported

(1):
- MUST register non-link-local address with router sending neighbor solicitations with the EARO field set
- MUST process neighbor advertisements
- node MUST NOT register its link-local address

(2):
- for router sol and adv, node MUST follow RFC6775 5.3,5.4 and RFC8505 5.6

(3):
-
