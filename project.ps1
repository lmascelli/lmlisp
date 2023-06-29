function build
{
  New-Item -Type Directory -Path build -ErrorAction Ignore
  Push-Location build
  cmake ..
  cmake --build .
  Pop-Location
}

function run
{
  param (
    $step
  )
  Push-Location build
  Invoke-Expression "./step$step*"
  Pop-Location
}

function _test
{
  param (
    $step
  )
  Copy-Item -Recurse -Force -Path build/* -Destination "~/data/projects/mal/impls/cpp"
  Push-Location "~/data/projects/mal"
  Invoke-Expression "make test^cpp^step$step"
  Pop-Location
}

switch ($args[0])
{
  "build"
  {
    build
  }

  "run"
  {
    build
    run $args[1]
  }

  "test"
  {
    _test $args[1]
  }

  default
  {
    build
  }
}
