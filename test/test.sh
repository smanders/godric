mkdir -p input
cd input
url=https://www2.census.gov/topics/genealogy/1990surnames
[ ! -f names_female.txt ] && wget -qO- ${url}/dist.female.first | awk '{print $1}' > names_female.txt
[ ! -f names_male.txt ] && wget -qO- ${url}/dist.male.first | awk '{print $1}' > names_male.txt
[ ! -f names_last.txt ] && wget -qO- ${url}/dist.all.last | awk '{print $1}' > names_last.txt
function getName()
{
  female=`shuf -n 1 names_female.txt`
  male=`shuf -n 1 names_male.txt`
  last=`shuf -n 1 names_last.txt`
  fini="$(echo ${female} | head -c 1)"
  mini="$(echo ${male} | head -c 1)"
  local name="${last}${mini}${fini}_${last}, ${male} & ${female}"
  echo "$name"
}
field1=(\
  "_UltraTax CS 12-31-"\
  )
field3=(\
  " ENGAGEMENT LETTER"\
  " Tax Documents"\
  " Tax Exemptions"\
  " Form 1040"\
  " Form 1040EZ"\
  " Form 1040A"\
  " Form W-2"\
  " Form W-4"\
  " Form W-4P"\
  " Form 1099-MISC"\
  " Form 1098"\
  )
names=100
until [ $names -lt 1 ]; do
  f0=$(getName)
  for f1 in "${field1[@]}"; do
    year=1990
    until [ $year -gt 2019 ]; do
      f2="${year}_${year}"
      for f3 in "${field3[@]}"; do
        touch "${f0}${f1}${f2}${f3}"
      done
      ((year++))
    done
  done
  ((names--))
done
