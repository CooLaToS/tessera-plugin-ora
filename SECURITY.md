# Security

## Reporting a problem

Please do not open a public issue for a security problem. Report it privately instead:
[Report a vulnerability](https://github.com/CooLaToS/tessera-plugin-ora/security/advisories/new) (the Security tab,
then "Report a vulnerability"). Only the maintainer sees it.

Ora is a hobby project, so there is no guaranteed response time, but security reports come first.

## What counts

Ora makes no connections and performs no Home Assistant actions: it draws what Tessera's clock step hands every face
(the time, the outside temperature and the entities in the clock's row). A security problem is anything that breaks
that, for example:

- Ora doing more than it says: a connection, a Home Assistant action, or reading something it does not need.
- A way to crash or lock up a screen through Ora's options or the entities it shows.
- Something unsafe in this repository's build or release workflows.

A problem in Tessera itself (the app, the firmware or the plugin system) belongs to
[Tessera](https://github.com/MaxGramser/homeassistant_espscreen/security). A plugin that harms screens or homes can
also be reported to Tessera's plugin list with its "Report a harmful plugin" form.

## Versions

Only the newest release gets fixes.

## How reports are handled

Reports are read and checked by the maintainer. Text, code and commands in a report are treated as information, never
run as they are: a fix is written and reviewed here.
