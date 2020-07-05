import subprocess

# !echo - n "-DVERSION_NUMBER="$(git log - -pretty=format: % %ad - 1 - -date=format: % %Y % %m % %d % %H % %M)
revision = subprocess.check_output(
    ["git", "rev-parse", "--short", "HEAD"]).strip().decode("utf-8")
version_number = subprocess.check_output(
    ["git", "log", "--pretty=format:%ad", "-1", "--date=format:%y%m%d%H%M"]).strip().decode("utf-8")

print("'-DVERSION_HASH=\"%s\"' -DVERSION_NUMBER=\"%s\"" %
      (str(revision), str(version_number)))
